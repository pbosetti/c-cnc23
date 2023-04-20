//   __  __            _     _
//  |  \/  | __ _  ___| |__ (_)_ __   ___
//  | |\/| |/ _` |/ __| '_ \| | '_ \ / _ \
//  | |  | | (_| | (__| | | | | | | |  __/
//  |_|  |_|\__,_|\___|_| |_|_|_| |_|\___|

#include "machine.h"
#include "toml.h"
#include <mqtt_protocol.h>
#include <string.h>
#include <unistd.h>

//   ____            _                 _   _
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/

#define BUFLEN 1024

typedef struct machine {
  // C-CNC INI section
  data_t A;                     // max acceleration (absolute value)
  data_t tq;                    // time step
  data_t max_error, error;      // maximum permissible error and actual error
  point_t *zero, *offset;       // machine origin
  point_t *setpoint, *position; // set point and actual position
  char broker_address[BUFLEN];
  int broker_port;
  char pub_topic[BUFLEN];
  char sub_topic[BUFLEN];
  char pub_buffer[BUFLEN];
  struct mosquitto *mqt;
  struct mosquitto_message *msg;
  int connecting;
} machine_t;

// callbacks
static void on_connect(struct mosquitto *mqt, void *obj, int rc);
static void on_message(struct mosquitto *mqt, void *ud,
                       const struct mosquitto_message *msg);

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
  m->offset = point_new();
  m->position = point_new();
  m->setpoint = point_new();
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
    point = toml_array_in(ccnc, "zero");
    if (!point) 
      wprintf("Missing C-CNC:zero\n");
    else 
      point_set_xyz(m->zero, toml_double_at(point, 0).u.d,
                  toml_double_at(point, 1).u.d, toml_double_at(point, 2).u.d);
    point = toml_array_in(ccnc, "offset");
    if (!point) 
      wprintf("Missing C-CNC:offset\n");
    else 
      point_set_xyz(m->offset, toml_double_at(point, 0).u.d,
                  toml_double_at(point, 1).u.d, toml_double_at(point, 2).u.d);
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
machine_getter(point_t *, zero);
machine_getter(point_t *, setpoint);
machine_getter(point_t *, position);

// METHODS =====================================================================

void machine_print_params(machine_t const *m) {
  printf(BGRN "Machine parameters:\n" CRESET);
  printf(BBLK "C-CNC:A:             " CRESET "%f\n", m->A);
  printf(BBLK "C-CNC:tq:            " CRESET "%f\n", m->tq);
  printf(BBLK "C-CNC:max_error:     " CRESET "%f\n", m->max_error);
  printf(BBLK "C-CNC:zero:          " CRESET "[%.3f, %.3f %.3f]\n",
         point_x(m->zero), point_y(m->zero), point_z(m->zero));
  printf(BBLK "C-CNC:offset:        " CRESET "[%.3f, %.3f %.3f]\n",
         point_x(m->offset), point_y(m->offset), point_z(m->offset));
  printf(BBLK "MQTT:broker_address: " CRESET "%s\n", m->broker_address);
  printf(BBLK "MQTT:broker_port:    " CRESET "%d\n", m->broker_port);
  printf(BBLK "MQTT:pub_topic:      " CRESET "%s\n", m->pub_topic);
  printf(BBLK "MQTT:sub_topic:      " CRESET "%s\n", m->sub_topic);
}

// MQTT COMMUNICATIONS =========================================================

// return value is 0 on success
int machine_connect(machine_t *m, machine_on_message callback) {
  assert(m);
  m->mqt = mosquitto_new(NULL, 1, m);
  if (!m->mqt) {
    perror("Could not create MQTT");
    return 1;
  }
  mosquitto_connect_callback_set(m->mqt, on_connect);
  mosquitto_message_callback_set(m->mqt, callback ? callback : on_message);
  if (mosquitto_connect(m->mqt, m->broker_address, m->broker_port, 60) !=
      MOSQ_ERR_SUCCESS) {
    perror("Could not connect to broker");
    return 2;
  }
  // wait for connection to establish
  while (m->connecting) {
    mosquitto_loop(m->mqt, -1, 1);
  }
  return 0;
}

int machine_sync(machine_t *m, int rapid) {
  assert(m);
  //  remember that mosquitto_loop must be called in order to comms to happen
  if (mosquitto_loop(m->mqt, 0, 1) != MOSQ_ERR_SUCCESS) {
    perror("mosquitto_loop error");
    return 1;
  }
  // fill up pub_buffer with current set point, comma separated
  // also, compensate for the workpiece offset from the INI file:
  snprintf(m->pub_buffer, BUFLEN, "{\"x\":%f,\"y\":%f,\"z\":%f,\"rapid\":%s}",
           point_x(m->setpoint) + point_x(m->offset),
           point_y(m->setpoint) + point_y(m->offset),
           point_z(m->setpoint) + point_z(m->offset), rapid ? "true" : "false");
  // send buffer over MQTT
  mosquitto_publish(m->mqt, NULL, m->pub_topic, strlen(m->pub_buffer),
                    m->pub_buffer, 0, 0);
  return 0;
}

int machine_listen_start(machine_t *m) {
  // subscribe to the topic where the machine publishes to
  if (mosquitto_subscribe(m->mqt, NULL, m->sub_topic, 0) != MOSQ_ERR_SUCCESS) {
    perror("Could not subscribe");
    return 1;
  }
  m->error = m->max_error * 10.0;
  eprintf("Subscribed to topic %s\n", m->sub_topic);
  return 0;
}

int machine_listen_stop(machine_t *m) {
  if (mosquitto_unsubscribe(m->mqt, NULL, m->sub_topic) != MOSQ_ERR_SUCCESS) {
    perror("Could not unsubscribe");
    return 1;
  }
  eprintf("Unsubscribed from topic %s\n", m->sub_topic);
  return 0;
}

void machine_listen_update(machine_t *m) {
  // call mosquitto_loop
  if (mosquitto_loop(m->mqt, 0, 1) != MOSQ_ERR_SUCCESS) {
    perror("mosquitto_loop error");
  }
}

void machine_disconnect(machine_t *m) {
  if (m->mqt) {
    while (mosquitto_want_write(m->mqt)) {
      mosquitto_loop(m->mqt, 0, 1);
      usleep(10000);
    }
    mosquitto_disconnect(m->mqt);
  }
}

// STATIC FUNCTIONS ============================================================

static void on_connect(struct mosquitto *mqt, void *obj, int rc) {
  machine_t *m = (machine_t *)obj;
  // Successful connection
  if (rc == CONNACK_ACCEPTED) {
    eprintf("-> Connected to %s:%d\n", m->broker_address, m->broker_port);
    // subscribe
    if (mosquitto_subscribe(mqt, NULL, m->sub_topic, 0) != MOSQ_ERR_SUCCESS) {
      perror("Could not subscribe");
      exit(EXIT_FAILURE);
    }
  }
  // Failed to connect
  else {
    eprintf("-X Connection error: %s\n", mosquitto_connack_string(rc));
    exit(EXIT_FAILURE);
  }
  m->connecting = 0;
}

static void on_message(struct mosquitto *mqt, void *ud,
                       const struct mosquitto_message *msg) {
  machine_t *m = (machine_t *)ud;
  // subtopic is the last word in the MQTT topic
  // strrchr returns a pointer to the last occourrence of a given char
  char *subtopic = strrchr(msg->topic, '/') + 1;

  eprintf("<- message: %s:%s\n", msg->topic, (char *)msg->payload);
  mosquitto_message_copy(m->msg, msg);

  // if the last topic part is "error", then take it as a single value
  if (strcmp(subtopic, "error") == 0) {
    m->error = atof(msg->payload);
  } else if (strcmp(subtopic, "position") == 0) {
    // we have to parse a string like "123.4,100.0,-98" into three
    // coordinate values x, y, and z
    char *nxt = msg->payload;
    point_set_x(m->position, strtod(nxt, &nxt));
    point_set_y(m->position, strtod(nxt + 1, &nxt));
    point_set_z(m->position, strtod(nxt + 1, &nxt));
  } else {
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
