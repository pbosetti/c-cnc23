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
  // C-CNC INI section
  data_t A;
  data_t tq;
  data_t max_error;
  point_t *zero;
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
    eprintf("Could not allocate memory for machine\n");
    return NULL;
  }
  memset(m, 0, sizeof(machine_t));
  // Defaults
  m->A = 100;
  m->max_error = 0.020;
  m->tq = 0.005;
  m->zero = point_new();
  point_set_xyz(m->zero, 0, 0, 0);

  ini_file = fopen(ini_path, "r");
  if (!ini_file) {
    eprintf("Could not open INI file: %s\n", strerror(errno));
    goto fail;
  }
  conf = toml_parse_file(ini_file, errbuf, BUFLEN);
  fclose(ini_file);
  if (!conf) {
    eprintf("Parsing INI file: %s\n", errbuf);
    goto fail;
  }

// macros for reading and storing INI values
#define TREAD_I(d, machine, tab, key)                                          \
  d = toml_int_in(tab, #key);                                                  \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s\n", toml_table_key(tab), #key);                     \
  else                                                                         \
    machine->key = d.u.i;
#define TREAD_D(d, machine, tab, key)                                          \
  d = toml_double_in(tab, #key);                                               \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s\n", toml_table_key(tab), #key);                     \
  else                                                                         \
    machine->key = d.u.d;
#define TREAD_S(d, machine, tab, key)                                          \
  d = toml_string_in(tab, #key);                                               \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s\n", toml_table_key(tab), #key);                     \
  else                                                                         \
    strncpy(machine->key, d.u.s, sizeof(machine->key));

  { // Section C-CNC
    toml_datum_t d;
    toml_table_t *ccnc = toml_table_in(conf, "C-CNC");
    if (!ccnc) {
      eprintf("Missing C-CNC section\n");
      goto fail;
    }
    TREAD_D(d, m, ccnc, A);
    TREAD_D(d, m, ccnc, max_error);
    TREAD_D(d, m, ccnc, tq);
  }
  toml_free(conf);
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
#define machine_getter(typ, par)                                               \
  typ machine_##par(const machine_t *m) {                                      \
    assert(m);                                                                 \
    return m->par;                                                             \
  }

machine_getter(data_t, A);
machine_getter(data_t, tq);
machine_getter(data_t, max_error);
machine_getter(point_t *, zero);

// Methods

void machine_print_params(machine_t *m) {
  printf(BGRN "Machine parameters:\n" CRESET);
  printf(BBLK "C-CNC:A:          " CRESET "%f\n", m->A);
  printf(BBLK "C-CNC:tq:         " CRESET "%f\n", m->tq);
  printf(BBLK "C-CNC:max_error:  " CRESET "%f\n", m->max_error);
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
