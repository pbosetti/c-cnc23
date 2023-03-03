//   __  __            _     _
//  |  \/  | __ _  ___| |__ (_)_ __   ___
//  | |\/| |/ _` |/ __| '_ \| | '_ \ / _ \
//  | |  | | (_| | (__| | | | | | | |  __/
//  |_|  |_|\__,_|\___|_| |_|_|_| |_|\___|
//
#include "machine.h"
#include "toml.h"
#include <errno.h>
#include <string.h>

//   ____            _                 _   _
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/

#define BUFLEN 1024

typedef struct machine {
  // MQTT INI section
  char broker_addr[BUFLEN];
  int broker_port;
  char pub_topic[BUFLEN];
  char sub_topic[BUFLEN];
  // C-CNC INI section
  data_t A;
  data_t tq;
  data_t max_error;
  data_t rt_pacing;
  // Other
  char pub_buffer[BUFLEN];
  data_t error;
} machine_t;

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// Lifecycle
machine_t *machine_new(const char *ini_path) {
  FILE *ini_file = NULL;
  char errbuf[BUFLEN];
  machine_t *m = NULL;
  toml_table_t *conf = NULL;

  m = malloc(sizeof(machine_t));
  if (!m) {
    eprintf("malloc");
    return NULL;
  }
  memset(m, 0, sizeof(machine_t));
  // Defaults
  m->A = 100;
  m->max_error = 0.020;
  m->tq = 0.005;
  m->rt_pacing = 0.25;
  strncpy(m->broker_addr, "localhost", BUFLEN);
  m->broker_port = 1883;
  strncpy(m->pub_topic, "c-cnc/setpoint", BUFLEN);
  strncpy(m->sub_topic, "c-cnc/status/#", BUFLEN);

  ini_file = fopen(ini_path, "r");
  if (!ini_file) {
    eprintf("fopen: %s", strerror(errno));
    goto fail;
  }
  conf = toml_parse_file(ini_file, errbuf, BUFLEN);
  fclose(ini_file);
  if (!conf) {
    eprintf("Parsing INI file: %s", errbuf);
    goto fail;
  }

// macros for reading and storing INI values
#define TREAD_I(d, m, tab, key, sec)                                           \
  d = toml_int_in(tab, #key);                                                  \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s", #sec, #key);                                      \
  else                                                                         \
    m->key = d.u.i;
#define TREAD_D(d, m, tab, key, sec)                                           \
  d = toml_double_in(tab, #key);                                               \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s", #sec, #key);                                      \
  else                                                                         \
    m->key = d.u.d;
#define TREAD_S(d, m, tab, key, sec)                                           \
  d = toml_string_in(tab, #key);                                               \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s", #sec, #key);                                      \
  else                                                                         \
    strncpy(m->key, d.u.s, sizeof(m->key));

  { // Section MQTT
    toml_datum_t d;
    toml_table_t *mqtt = toml_table_in(conf, "MQTT");
    if (!mqtt) {
      eprintf("Missing MQTT section");
      goto fail;
    }
    // Values should be read like this:
    // d = toml_string_in(mqtt, "broker_addr");
    // if (!d.ok) error("Missing MQTT:broker_addr", NULL);
    // else strncpy(m->broker_addr, d.u.s, sizeof(m->broker_addr));
    // To make this DRY, we use macros (see above)
    TREAD_S(d, m, mqtt, broker_addr, "MQTT");
    TREAD_I(d, m, mqtt, broker_port, "MQTT");
    TREAD_S(d, m, mqtt, sub_topic, "MQTT");
    TREAD_S(d, m, mqtt, pub_topic, "MQTT");
  }
  { // Section C-CNC
    toml_datum_t d;
    toml_table_t *ccnc = toml_table_in(conf, "C-CNC");
    if (!ccnc) {
      eprintf("Missing C-CNC section");
      goto fail;
    }
    TREAD_D(d, m, ccnc, A, "C-CNC");
    TREAD_D(d, m, ccnc, max_error, "C-CNC");
    TREAD_D(d, m, ccnc, tq, "C-CNC");
    TREAD_D(d, m, ccnc, rt_pacing, "C-CNC");
  }
  return m;

fail:
  free(m);
  return NULL;
}

void machine_free(machine_t *m) {
  assert(m);
  free(m);
}

// MARK: Accessors

// Methods

void machine_print_params(machine_t *m) {
  printf(BGRN "Machine parameters:\n" CRESET);
  printf(BBLK "MQTT:broker:      " CRESET "%s:%d\n", m->broker_addr,
         m->broker_port);
  printf(BBLK "MQTT:pub_topic:   " CRESET "%s\n", m->pub_topic);
  printf(BBLK "MQTT:sub_topic:   " CRESET "%s\n", m->sub_topic);
  printf(BBLK "C-CNC:A:          " CRESET "%f\n", m->A);
  printf(BBLK "C-CNC:tq:         " CRESET "%f\n", m->tq);
  printf(BBLK "C-CNC:max_error:  " CRESET "%f\n", m->max_error);
  printf(BBLK "C-CNC:rt_pacing:  " CRESET "%f\n", m->rt_pacing);
}

#ifdef MACHINE_MAIN
//   _____         _   _
//  |_   _|__  ___| |_(_)_ __   __ _
//    | |/ _ \/ __| __| | '_ \ / _` |
//    | |  __/\__ \ |_| | | | | (_| |
//    |_|\___||___/\__|_|_| |_|\__, |
// Only for testing local file |___/

int main(int argc, const char **argv) {
  machine_t *m = machine_new(argv[1]);
  if (!m)
    return 1;
  machine_print_params(m);
  machine_free(m);
}

#endif
