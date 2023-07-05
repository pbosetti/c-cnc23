#include "../axis.h"
#include "../defines.h"
#include "../toml.h"
#include <mosquitto.h>
#include <mqtt_protocol.h>

#include <math.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define INI_FILE "machine.ini"
#define BUFLEN 1024

int _running = 1;

typedef struct {
  axis_t *ax, *ay, *az;
  data_t dt;
  char broker_addr[BUFLEN];
  int broker_port;
  char sub_topic[BUFLEN];
  char pub_topic_err[BUFLEN];
  char pub_topic_pos[BUFLEN];
  struct mosquitto *mqtt;
  int connecting;
  int rapid;
  int program_run;
} sim_t;

// empty signal handler
void alrm_handler(int signal) {}
void int_handler(int signal) { _running = 0; }

sim_t *sim_new(char *ini_path) {
  FILE *ini_file = NULL;
  toml_table_t *conf = NULL, *sec = NULL;
  toml_datum_t datum;
  char errbuf[BUFLEN];
  sim_t *sim = malloc(sizeof(sim_t));
  memset(sim, 0, sizeof(*sim));
  sim->connecting = 1;
  sim->rapid = 0;

  // init axes
  sim->ax = axis_new(ini_path, "X");
  sim->ay = axis_new(ini_path, "Y");
  sim->az = axis_new(ini_path, "Z");
  if (!sim->ax || !sim->ay || !sim->az) {
    goto fail;
  }
  // read config
  ini_file = fopen(ini_path, "r");
  if (!ini_file) {
    eprintf("Could not open the file %s\n", ini_path);
    goto fail;
  }
  conf = toml_parse_file(ini_file, errbuf, BUFLEN);
  fclose(ini_file);
  if (!conf) {
    eprintf("Could not parse INI file: %s\n", errbuf);
    goto fail;
  }
#define T_READ_I(section, key, field)                                          \
  datum = toml_int_in(sec, key);                                               \
  if (!datum.ok)                                                               \
    wprintf("Missing %s:%s\n", section, key);                                  \
  else                                                                         \
    sim->field = datum.u.i;
#define T_READ_D(section, key, field)                                          \
  datum = toml_double_in(sec, key);                                            \
  if (!datum.ok)                                                               \
    wprintf("Missing %s:%s\n", section, key);                                  \
  else                                                                         \
    sim->field = datum.u.d;
#define T_READ_S(section, key, field)                                          \
  datum = toml_string_in(sec, key);                                            \
  if (!datum.ok)                                                               \
    wprintf("Missing %s:%s\n", section, key);                                  \
  else {                                                                       \
    strncpy(sim->field, datum.u.s, BUFLEN);                                    \
    free(datum.u.s);                                                           \
  }

  sec = toml_table_in(conf, "C-CNC");
  if (!sec) {
    eprintf("Missing C-CNC section\n");
    goto fail;
  }
  T_READ_D("C-CNC", "tq", dt);
  sim->dt *= 1E6;

  sec = toml_table_in(conf, "MQTT");
  if (!sec) {
    eprintf("Missing MQTT section\n");
    goto fail;
  }
  T_READ_I("MQTT", "broker_port", broker_port);
  T_READ_S("MQTT", "broker_address", broker_addr);
  T_READ_S("MQTT", "sub_topic", pub_topic_err);
  T_READ_S("MQTT", "pub_topic", sub_topic);
  sim->pub_topic_err[strlen(sim->pub_topic_err) - 1] = '\0';
  strcpy(sim->pub_topic_pos, sim->pub_topic_err);
  strcat(sim->pub_topic_err, "error");
  strcat(sim->pub_topic_pos, "position");

  toml_free(conf);
  return sim;

fail:
  return NULL;
}

static void on_connect(struct mosquitto *m, void *obj, int rc) {
  sim_t *sim = (sim_t *)obj;
  if (rc == CONNACK_ACCEPTED) {
    wprintf("Connected to %s:%d\n", sim->broker_addr, sim->broker_port);
    // subscribe to topic
    if (mosquitto_subscribe(sim->mqtt, NULL, sim->sub_topic, 0) ==
        MOSQ_ERR_SUCCESS) {
      wprintf("Subscribed to %s\n", sim->sub_topic);
    } else {
      perror("Could not subscribe");
      exit(EXIT_FAILURE);
    }
  }
  // fail to connect
  else {
    eprintf("Conection error: %s\n", mosquitto_connack_string(rc));
    exit(EXIT_FAILURE);
  }
  sim->connecting = 0;
}

static void on_message(struct mosquitto *m, void *obj,
                       const struct mosquitto_message *msg) {
  sim_t *sim = (sim_t *)obj;
  char *substr = NULL;
  data_t x, y, z;
  sim->program_run = 1;
  if (strcmp(msg->topic, sim->sub_topic) == 0) {
    substr = strchr(msg->payload, 'x');
    x = atof(substr + 3) / 1000.0;
    substr = strchr(substr + 4, 'y');
    y = atof(substr + 3) / 1000.0;
    substr = strchr(substr + 4, 'z');
    z = atof(substr + 3) / 1000.0;
    substr = strchr(substr + 4, 'd');
    sim->rapid = atoi(substr + 3);
    axis_set_setpoint(sim->ax, x);
    axis_set_setpoint(sim->ay, y);
    axis_set_setpoint(sim->az, z);
  }
}

//   __  __    _    ___ _   _
//  |  \/  |  / \  |_ _| \ | |
//  | |\/| | / _ \  | ||  \| |
//  | |  | |/ ___ \ | || |\  |
//  |_|  |_/_/   \_\___|_| \_|

int main(int argc, char const *argv[]) {
  sim_t *sim = sim_new(INI_FILE);
  axis_t *ax, *ay, *az;
  data_t x, sx, y, sy, z, sz, delta;
  struct mosquitto *mqtt = NULL;
  FILE *logfile = NULL;
  char payload[BUFLEN];
  int count = 0;
  if (!sim) {
    eprintf("Could not create simulator\n");
    return EXIT_FAILURE;
  }
  ax = sim->ax;
  ay = sim->ay;
  az = sim->az;

  // Setup timing facility
  {
    struct itimerval itv;
    // set the timer intervals
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = sim->dt;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = sim->dt;

    // install signal handler
    signal(SIGALRM, alrm_handler);
    signal(SIGINT, int_handler);

    // load the timer mechanism
    if (setitimer(ITIMER_REAL, &itv, NULL)) {
      eprintf("Could not set timer\n");
      exit(EXIT_FAILURE);
    }
  }

  // Log file
  if (argc == 2) {
    if (!(logfile = fopen(argv[1], "w"))) {
      eprintf("Cannot open %s for writing\n", argv[1]);
      exit(EXIT_FAILURE);
    }
  }

  // Setup comms
  mosquitto_lib_init();
  mqtt = mosquitto_new(NULL, 1, sim);
  sim->mqtt = mqtt;
  if (!mqtt) {
    perror(BRED "Could not create MQTT" CRESET);
    return EXIT_FAILURE;
  }
  mosquitto_connect_callback_set(mqtt, on_connect);
  mosquitto_message_callback_set(mqtt, on_message);
  if (mosquitto_connect(mqtt, sim->broker_addr, sim->broker_port, 10) !=
      MOSQ_ERR_SUCCESS) {
    perror(BRED "Invalid broker connection parameters" CRESET);
    return EXIT_FAILURE;
  }
  // wait for the connection to be established
  while (sim->connecting) {
    mosquitto_loop(mqtt, -1, 1);
    if (++count >= 5) {
      eprintf("Could not connect to broker\n");
      return EXIT_FAILURE;
    }
  }

  // Setup axes
  axis_set_torque(ax, 0);
  axis_set_torque(ay, 0);
  axis_set_torque(az, 0);
  axis_link(ay, az);
  axis_link(ax, ay);

  axis_reset(ax, 0);
  axis_reset(ay, 0);
  axis_reset(az, 0);
  axis_set_setpoint(ax, 0);
  axis_set_setpoint(ay, 0);
  axis_set_setpoint(az, 0);

#define LOG_HEADER                                                             \
  "        t        qx        sx        x         vx        qy        sy     " \
  "   y         vy        qz        sz        z         vz        d  r\n"
  printf(LOG_HEADER);
  if (logfile) {
    fprintf(logfile, LOG_HEADER);
  }

  axis_run(ax);
  axis_run(ay);
  axis_run(az);

  // Timing loop
  while (_running) {
    axis_pid(ax);
    axis_pid(ay);
    axis_pid(az);
    sx = axis_setpoint(ax) * 1000;
    sy = axis_setpoint(ay) * 1000;
    sz = axis_setpoint(az) * 1000;
    x = axis_position(ax) * 1000;
    y = axis_position(ay) * 1000;
    z = axis_position(az) * 1000;
    delta = sqrt(pow(x - sx, 2) + pow(y - sy, 2) + pow(z - sz, 2));
    printf("\r");
    printf("%9.4f " RED "%9.3f %9.3f %9.3f %9.3f " GRN
           "%9.3f %9.3f %9.3f %9.3f " BLU "%9.3f %9.3f %9.3f %9.3f" YEL
           " %9.3f" CRESET " %c",
           axis_time(ax), axis_torque(ax), sx, x, axis_speed(ax),
           axis_torque(ay), sy, y, axis_speed(ay), axis_torque(az), sz, z,
           axis_speed(az), delta, sim->rapid ? 'R' : 'I');
    if (logfile && sim->program_run) {
      fprintf(logfile,
              "%f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f "
              "%9.3f %9.3f %9.3f %c\n",
              axis_time(ax), axis_torque(ax), sx, x, axis_speed(ax),
              axis_torque(ay), sy, y, axis_speed(ay), axis_torque(az), sz, z,
              axis_speed(az), delta, sim->rapid ? 'R' : 'I');
    }
    fflush(stdout);
    if (sim->program_run) {
      sprintf(payload, "%f", delta);
      mosquitto_publish(sim->mqtt, NULL, sim->pub_topic_err, strlen(payload),
                        payload, 0, 0);
      sprintf(payload, "%f,%f,%f", x, y, z);
      mosquitto_publish(sim->mqtt, NULL, sim->pub_topic_pos, strlen(payload),
                        payload, 0, 0);
    }
    mosquitto_loop(mqtt, -1, 1);
    usleep(sim->dt * 10);
  }

  printf("\n\nExiting...\n");
  // Finalize
  if (logfile) fclose(logfile);
  axis_stop(ax);
  axis_stop(ay);
  axis_stop(az);
  usleep(500000);
  axis_free(ax);
  axis_free(ay);
  axis_free(az);
  mosquitto_destroy(sim->mqtt);
  mosquitto_lib_cleanup();
  free(sim);
  printf("done.\n");
  return 0;
}
