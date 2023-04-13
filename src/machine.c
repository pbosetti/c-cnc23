//   __  __            _     _
//  |  \/  | __ _  ___| |__ (_)_ __   ___
//  | |\/| |/ _` |/ __| '_ \| | '_ \ / _ \
//  | |  | | (_| | (__| | | | | | | |  __/
//  |_|  |_|\__,_|\___|_| |_|_|_| |_|\___|

#include "machine.h"
#include "toml.h"
#include <string.h>

//   ____            _                 _   _
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/

#define BUFLEN 1024

typedef struct machine {
  data_t A;
  data_t tq;
  data_t max_error, error;
  point_t *zero;
  point_t *setpoint, *position;
} machine_t;

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// LIFECYCLE

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
  point_set_xyz(m->zero, 0, 0, 0);

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
    strncpy(machine->key, d.u.s, strlen(machine->key));                        \
    free(d.u.s);                                                               \
  }

  // 3. extract values from the C-CNC section
  // Sections must exist; missing keys only give a warning and use the default
  {
    toml_datum_t d;
    toml_table_t *ccnc = toml_table_in(conf, "C-CNC");
    if (!ccnc) {
      eprintf("Missing C-CNC section\n");
      goto fail;
    }
    T_READ_D(d, m, ccnc, A);
    T_READ_D(d, m, ccnc, max_error);
    T_READ_D(d, m, ccnc, tq);
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

// ACCESSORS
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

// METHODS
void machine_print_params(machine_t const *m) {
  printf(BGRN "Machine parameters:\n" CRESET);
  printf(BBLK "C-CNC:A:         " CRESET "%f\n", m->A);
  printf(BBLK "C-CNC:tq:        " CRESET "%f\n", m->tq);
  printf(BBLK "C-CNC:max_error: " CRESET "%f\n", m->max_error);
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