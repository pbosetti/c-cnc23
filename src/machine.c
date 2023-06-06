//   __  __            _     _
//  |  \/  | __ _  ___| |__ (_)_ __   ___
//  | |\/| |/ _` |/ __| '_ \| | '_ \ / _ \
//  | |  | | (_| | (__| | | | | | | |  __/
//  |_|  |_|\__,_|\___|_| |_|_|_| |_|\___|

#include "machine.h"
#include "toml.h"
#include <string.h>
#include <mqtt_protocol.h>
#include <unistd.h> // for usleep()

//   ____            _                 _   _
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/

#define BUFLEN 1024

typedef struct machine {
  data_t A;                      // max acceleration/deceleration
  data_t tq;                     // sampling time
  data_t max_error, error;       // maximum error and current error
  data_t fmax;                   // maximum feed rate
  point_t *zero;                 // machine origin
  point_t *setpoint, *position;  // set point and current position
  point_t *offset;               // offset of the workpiece reference frame
  char broker_address[BUFLEN];   // internet address of MQTT broker
  int broker_port;               // port of MQTT broker
  char pub_topic[BUFLEN];        // topic where to publish the set point
  char sub_topic[BUFLEN];        // topic where current position is published
  char pub_buffer[BUFLEN];       // buffer for storing the payload
  struct mosquitto *mqt;         // mosquitto object
  struct mosquitto_message *msg; // mosquitto message structure
  int connecting;                // 1 when disconnected or about to connect
  data_t rt_pacing;                 // real time scaling
} machine_t;

// Callbacks
static void on_connect(struct mosquitto *m, void *obj, int rc);
static void on_message(struct mosquitto *m, void *obj, const struct mosquitto_message *msg);


//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// LIFECYCLE ===================================================================

machine_t *machine_new(char const *cfg_path) {
  machine_t *m = NULL;
  FILE *ini_file = NULL;
  toml_table_t *conf = NULL;
  char errbuf[BUFLEN];
  // Allocate memory
  m = malloc(sizeof(*m));
  if (!m) {
    eprintf("Could not allocate memory for machine object\n");
    return NULL;
  }
  // Set defaults:
  memset(m, 0, sizeof(*m));
  m->A = 100;
  m->max_error = 0.010;
  m->tq = 0.005;
  m->zero = point_new();
  m->position = point_new();
  m->setpoint = point_new();
  m->offset = point_new();
  point_set_xyz(m->zero, 0, 0, 0);
  point_set_xyz(m->offset, 0, 0, 0);

  // Import values form a INI file
  // 1. open the file
  ini_file = fopen(cfg_path, "r");
  if (!ini_file) {
    eprintf("Could not open the file %s\n", cfg_path);
    goto fail;
  }
  // 2. parse the file into a toml_table_t object
  conf = toml_parse_file(ini_file, errbuf, BUFLEN);
  fclose(ini_file);
  if (!conf) {
    eprintf("Could not parse INI file: %s\n", errbuf);
    goto fail;
  }

  // define some macros for reading doubles, integers and strings.
#define T_READ_I(d, machine, tab, key)                                         \
  d = toml_int_in(tab, #key);                                                  \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s\n", toml_table_key(tab), #key);                     \
  else                                                                         \
    machine->key = d.u.i;
#define T_READ_D(d, machine, tab, key)                                         \
  d = toml_double_in(tab, #key);                                               \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s\n", toml_table_key(tab), #key);                     \
  else                                                                         \
    machine->key = d.u.d;
#define T_READ_S(d, machine, tab, key)                                         \
  d = toml_string_in(tab, #key);                                               \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s\n", toml_table_key(tab), #key);                     \
  else {                                                                       \
    strncpy(machine->key, d.u.s, BUFLEN);                                      \
    free(d.u.s);                                                               \
  }

  // 3. extract values from the C-CNC section
  // Sections must exist; missing keys only give a warning and use the default
  {
    toml_datum_t d;
    toml_array_t *point;
    toml_table_t *ccnc = toml_table_in(conf, "C-CNC");
    if (!ccnc) {
      eprintf("Missing C-CNC section\n");
      goto fail;
    }
    T_READ_D(d, m, ccnc, A);
    T_READ_D(d, m, ccnc, max_error);
    T_READ_D(d, m, ccnc, tq);
    T_READ_D(d, m, ccnc, fmax);
    T_READ_D(d, m, ccnc, rt_pacing);
    // WP origin
    point = toml_array_in(ccnc, "offset");
    if (!point) {
      wprintf("Missing C-CNC:offset, using default");
    } else {
      point_set_xyz(m->offset, 
        toml_double_at(point, 0).u.d,
        toml_double_at(point, 1).u.d,
        toml_double_at(point, 2).u.d
      );
    }
    // Machine initial position
    point = toml_array_in(ccnc, "zero");
    if (!point) {
      wprintf("Missing C-CNC:zero, using default");
    } else {
      point_set_xyz(m->zero, 
        toml_double_at(point, 0).u.d,
        toml_double_at(point, 1).u.d,
        toml_double_at(point, 2).u.d
      );
    }
  }
  {
    toml_datum_t d;
    toml_table_t *mqtt = toml_table_in(conf, "MQTT");
    if (!mqtt) {
      eprintf("Missing MQTT section\n");
      goto fail;
    }
    T_READ_S(d, m, mqtt, broker_address);
    T_READ_I(d, m, mqtt, broker_port);
    T_READ_S(d, m, mqtt, pub_topic);
    T_READ_S(d, m, mqtt, sub_topic);
  }
  toml_free(conf);
  if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS) {
    eprintf("Could not initialize the mosquitto library\n");
    goto fail;
  }
  m->connecting = 1;
  return m;

fail:
  machine_free(m);
  return NULL;
}

void machine_free(machine_t *m) {
  assert(m);
  if (m->zero)
    point_free(m->zero);
  if (m->setpoint)
    point_free(m->setpoint);
  if (m->position)
    point_free(m->position);
  if (m->mqt)
    mosquitto_destroy(m->mqt);
  mosquitto_lib_cleanup();
  free(m);
}


// ACCESSORS ===================================================================

#define machine_getter(typ, par)                                               \
  typ machine_##par(machine_t const *m) {                                      \
    assert(m);                                                                 \
    return m->par;                                                             \
  }

machine_getter(data_t, A);
machine_getter(data_t, tq);
machine_getter(data_t, max_error);
machine_getter(data_t, error);
machine_getter(data_t, fmax);
machine_getter(data_t, rt_pacing);
machine_getter(point_t *, zero);
machine_getter(point_t *, setpoint);
machine_getter(point_t *, position);


// METHODS =====================================================================

void machine_print_params(machine_t const *m) {
  fprintf(stderr, BGRN "Machine parameters:\n" CRESET);
  // C-CNC section
  fprintf(stderr, BBLK "C-CNC:A:          " CRESET "%f\n", m->A);
  fprintf(stderr, BBLK "C-CNC:tq:         " CRESET "%f\n", m->tq);
  fprintf(stderr, BBLK "C-CNC:max_error:  " CRESET "%f\n", m->max_error);
  fprintf(stderr, BBLK "C-CNC:fmax:       " CRESET "%f\n", m->fmax);
  fprintf(stderr, BBLK "C-CNC:zero        " CRESET "[%.3f, %.3f, %.3f]\n", 
    point_x(m->zero), point_y(m->zero), point_z(m->zero));
  fprintf(stderr, BBLK "C-CNC:rt_pacing:  " CRESET "%f\n", m->rt_pacing);
  fprintf(stderr, BBLK "C-CNC:offset      " CRESET "[%.3f, %.3f, %.3f]\n", 
    point_x(m->offset), point_y(m->offset), point_z(m->offset));
  // MQTT section
  fprintf(stderr, BBLK "MQTT:broker_addr: " CRESET "%s\n", m->broker_address);
  fprintf(stderr, BBLK "MQTT:broker_port: " CRESET "%d\n", m->broker_port);
  fprintf(stderr, BBLK "MQTT:pub_topic:   " CRESET "%s\n", m->pub_topic);
  fprintf(stderr, BBLK "MQTT:sub_topic:   " CRESET "%s\n", m->sub_topic);
}


int machine_connect(machine_t *m, machine_on_message callback) {
  assert(m);
  m->mqt = mosquitto_new(NULL, 1, m);
  if (!m->mqt) {
    perror(BRED"Could not create MQTT"CRESET);
    return EXIT_FAILURE;
  }
  mosquitto_connect_callback_set(m->mqt, on_connect);
  mosquitto_message_callback_set(m->mqt, callback ? callback : on_message);
  mosquitto_loop_start(m->mqt);
  if (mosquitto_connect(m->mqt, m->broker_address, m->broker_port, 10) != MOSQ_ERR_SUCCESS) {
    perror(BRED"Invalid broker connection parameters"CRESET);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int machine_sync(machine_t *m, int rapid) {
  assert(m && m->mqt);
  // Fill up m->pub_buffer with the set point in JSON format
  // {"x":100.2, "y":123, "z":0.0, "rapid":false}
  snprintf(m->pub_buffer, BUFLEN, "{\"x\":%f, \"y\":%f, \"z\":%f, \"rapid\":%d}",
    point_x(m->setpoint) + point_x(m->offset),
    point_y(m->setpoint) + point_y(m->offset),
    point_z(m->setpoint) + point_z(m->offset),
    rapid ? 1 : 0
  );
  // send the buffer:
  if (mosquitto_publish(m->mqt, NULL, m->pub_topic, strlen(m->pub_buffer), m->pub_buffer, 0, 0) != MOSQ_ERR_SUCCESS) {
    perror(BRED"Could not sent message"CRESET);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int machine_listen_start(machine_t *m) {
  assert(m && m->mqt);
  if (mosquitto_subscribe(m->mqt, NULL, m->sub_topic, 0) != MOSQ_ERR_SUCCESS) {
    perror(BRED"Could not subscribe"CRESET);
    return EXIT_FAILURE;
  }
  m->error = m->max_error * 10.0;
  // wprintf("Subscribed to topic %s\n", m->sub_topic);
  return EXIT_SUCCESS;
}

int machine_listen_stop(machine_t *m) {
  assert(m && m->mqt);
  if (mosquitto_unsubscribe(m->mqt, NULL, m->sub_topic) != MOSQ_ERR_SUCCESS) {
    perror(BRED"Could not unsubscribe"CRESET);
    return EXIT_FAILURE;
  }
  // wprintf("Unsubscribed from topic %s\n", m->sub_topic);
  return EXIT_SUCCESS;
}

void machine_disconnect(machine_t *m) {
  assert(m && m->mqt);
  while (mosquitto_want_write(m->mqt)) {
    usleep(10000);
  }
  mosquitto_loop_stop(m->mqt, 1);
  mosquitto_disconnect(m->mqt);
  m->connecting = 1;
}


// Static functions
static void on_connect(struct mosquitto *mqt, void *obj, int rc) {
  // first of all, obj is set to be a machine_t * object when we called 
  // mosquitto_new in machine_connect: we need to cast it back to a new pointer
  // before using it:
  machine_t *m = (machine_t *)obj;
  // check rc: if CONNACK_ACCEPTED, then the connection succeeded
  if (rc == CONNACK_ACCEPTED) {
    wprintf("-> Connected to %s:%d\n\r", m->broker_address, m->broker_port);
    // subscribe to topic
    if (mosquitto_subscribe(mqt, NULL, m->sub_topic, 0) != MOSQ_ERR_SUCCESS) {
      perror("Could not subsccribe");
      exit(EXIT_FAILURE);
    }
  }
  // fail to connect
  else {
    eprintf("-X Conection error: %s\n", mosquitto_connack_string(rc));
    exit(EXIT_FAILURE);
  }
  m->connecting = 0;
}

// messages arrive on the topic c-cnc/status/#
static void on_message(struct mosquitto *m, void *obj, const struct mosquitto_message *msg) {
  machine_t *machine = (machine_t *)obj;
  // find out the topic subpath, by splitting the topic on / and only taking 
  // the last one:
  // e.g.: "c-cnc/status/error" -> "/error", + 1 -> "error"
  char *subtopic = strrchr(msg->topic, '/') + 1;
  // fprintf(stderr, "<- message: %s:%s\n", msg->topic, (char *)msg->payload);

  // make a copy of the message for storing it into machine
  mosquitto_message_copy(machine->msg, msg);

  // act accoring to the last part of the topic:
  // c-cnc/status/error
  if (strcmp(subtopic, "error") == 0) {
    machine->error = atof(msg->payload);
  }
  // c-cnc/status/position
  else if (strcmp(subtopic, "position") == 0) {
    // we get a message as "123.5,0.100,200"
    char *nxt = msg->payload;
    point_set_x(machine->position, strtod(nxt, &nxt)); // ",0.100,200"
    point_set_y(machine->position, strtod(nxt + 1, &nxt)); // ",200"
    point_set_z(machine->position, strtod(nxt + 1, &nxt)); // ""
  }
  else {
    eprintf("Got unexpected message on %s\n", msg->topic);
  }
}



//   _____         _
//  |_   _|__  ___| |_
//    | |/ _ \/ __| __|
//    | |  __/\__ \ |_
//    |_|\___||___/\__|

#ifdef MACHINE_MAIN

int main(int argc, char const *argv[]) {
  machine_t *m = machine_new(argv[1]);
  if (!m)
    exit(EXIT_FAILURE);
  machine_print_params(m);
  machine_free(m);
  return 0;
}

#endif