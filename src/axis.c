//      _          _
//     / \   __  _(_)___
//    / _ \  \ \/ / / __|
//   / ___ \  >  <| \__ \
//  /_/   \_\/_/\_\_|___/

#include "axis.h"
#include "toml.h"
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define NAME_LENGTH 3
#define BUFLEN 1024

typedef struct axis {
  char name[NAME_LENGTH]; // like "X1\0"
  data_t time;            // last time for dynamics evaluation
  data_t position, speed; // ditto
  data_t setpoint;        // ditto
  data_t length;          // ditto
  data_t friction;        // ditto
  data_t mass;
  data_t effective_mass;
  data_t torque, max_torque;
  data_t pitch;              // conversion from torque to thrust
  data_t gravity;            // acceleration (vertical axes)
  data_t p, i, d;            // PID parameters
  data_t err_i, err_d;
  data_t prev_error;         // PID error at previous step
  data_t prev_ierror;        // PID previous integral error
  data_t prev_time;          // time at previous step
  struct axis *linked;       // another axis connected to this one
  pthread_t thread;          // Euler integration thread
  int stop, running;         // stop request for thread and thread status
  time_t t0;                 // time at axis creation (for delta)
  useconds_t integration_dt; // time step in Euler integration loop
} axis_t;

//   _     _  __                      _
//  | |   (_)/ _| ___  ___ _   _  ___| | ___
//  | |   | | |_ / _ \/ __| | | |/ __| |/ _ \
//  | |___| |  _|  __/ (__| |_| | (__| |  __/
//  |_____|_|_|  \___|\___|\__, |\___|_|\___|
//                         |___/

axis_t *axis_new(char const *ini_path, char const *name) {
  FILE *ini_file = NULL;
  toml_table_t *conf = NULL;
  char errbuf[BUFLEN];
  axis_t *axis = malloc(sizeof(axis_t));
  if (!axis) {
    goto fail;
  }

  // set defaults
  memset(axis, 0, sizeof(axis_t));
  strncpy(axis->name, name, NAME_LENGTH);
  axis->length = 1;
  axis->mass = 1000;
  axis->effective_mass = axis->mass;
  axis->friction = 100;
  axis->pitch = 0.1;
  axis->max_torque = 10;
  axis->p = 1;

  // Import values form a INI file
  // 1. open the file
  ini_file = fopen(ini_path, "r");
  if (!ini_file) {
    eprintf("Could not open the file %s\n", ini_path);
    goto fail;
  }
  // 2. parse the file into a toml_table_t object
  conf = toml_parse_file(ini_file, errbuf, BUFLEN);
  if (!conf) {
    eprintf("Could not parse INI file: %s\n", errbuf);
    goto fail;
  }
  fclose(ini_file);

  // define some macros for reading doubles, integers and strings.
#define T_READ_D(d, axis, tab, key)                                            \
  d = toml_double_in(tab, #key);                                               \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s (default %f)\n", toml_table_key(tab), #key,         \
            axis->key);                                                        \
  else                                                                         \
    axis->key = d.u.d;
#define T_READ_I(d, axis, tab, key)                                            \
  d = toml_int_in(tab, #key);                                                  \
  if (!d.ok)                                                                   \
    wprintf("Missing %s:%s (default %d)\n", toml_table_key(tab), #key,         \
            axis->key);                                                        \
  else                                                                         \
    axis->key = d.u.i;

  // 3. extract values from the C-CNC section
  // Sections must exist; missing keys only give a warning and use the default
  {
    toml_datum_t d;
    toml_table_t *tab = toml_table_in(conf, name);
    if (!tab) {
      eprintf("Missing %s section\n", name);
      goto fail;
    }
    T_READ_D(d, axis, tab, length);
    T_READ_D(d, axis, tab, friction);
    T_READ_D(d, axis, tab, mass);
    T_READ_D(d, axis, tab, max_torque);
    T_READ_D(d, axis, tab, pitch);
    T_READ_D(d, axis, tab, gravity);
    T_READ_D(d, axis, tab, p);
    T_READ_D(d, axis, tab, i);
    T_READ_D(d, axis, tab, d);
    T_READ_I(d, axis, tab, integration_dt);
  }

  toml_free(conf);
  return axis;

fail:
  toml_free(conf);
  fclose(ini_file);
  axis_free(axis);
  return NULL;
}

void axis_free(axis_t *axis) {
  if (!axis) {
    return;
  }
  free(axis);
}

void axis_link(axis_t *master, axis_t *slave) {
  master->linked = slave;
  master->effective_mass = master->mass + slave->effective_mass;
}

//      _
//     / \   ___ ___ ___  ___ ___  ___  _ __ ___
//    / _ \ / __/ __/ _ \/ __/ __|/ _ \| '__/ __|
//   / ___ \ (_| (_|  __/\__ \__ \ (_) | |  \__ \
//  /_/   \_\___\___\___||___/___/\___/|_|  |___/

#define axis_getter(typ, par)                                                  \
  typ axis_##par(axis_t const *b) {                                            \
    assert(b);                                                                 \
    return b->par;                                                             \
  }

#define axis_setter(typ, par)                                                  \
  void axis_set_##par(axis_t *b, typ value) {                                  \
    assert(b);                                                                 \
    b->par = value;                                                            \
  }

// Getters
axis_getter(char const *, name);
axis_getter(data_t, position);
axis_getter(data_t, speed);
axis_getter(data_t, time);
axis_getter(data_t, setpoint);
axis_getter(data_t, length);
axis_getter(data_t, torque);
axis_getter(data_t, max_torque);
axis_getter(axis_t *, linked);
axis_getter(int, running);

// Setters
axis_setter(data_t, torque);
axis_setter(data_t, setpoint);
axis_setter(data_t, i);

//   ____                              _
//  |  _ \ _   _ _ __   __ _ _ __ ___ (_) ___ ___
//  | | | | | | | '_ \ / _` | '_ ` _ \| |/ __/ __|
//  | |_| | |_| | | | | (_| | | | | | | | (__\__ \
//  |____/ \__, |_| |_|\__,_|_| |_| |_|_|\___|___/
//         |___/

void axis_reset(axis_t *axis, data_t position) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  axis->position = position;
  axis->t0 = tv.tv_sec;
  axis->prev_time = axis->time;
  axis->prev_error = axis->setpoint - axis->position;
  axis->speed = 0.0;
}

void axis_pid(axis_t *axis) {
  data_t out, err, dt;
  err = axis->setpoint - axis->position;
  dt = axis->time - axis->prev_time;
  if (axis->i)
    axis->err_i += (err + axis->prev_error) * dt / 2.0;
  if (axis->d && dt > 0)
    axis->err_d = (err - axis->prev_error) / dt;
  axis->prev_error = err;
  axis->prev_time = axis->time;
  out = axis->p * err + axis->i * axis->err_i + axis->d * axis->err_d;
  if (out > 0)
    axis->torque = fmin(out, axis->max_torque);
  else
    axis->torque = fmax(out, -axis->max_torque);
}

void axis_forward_integrate(axis_t *a, data_t t) {
  data_t dt = t - a->time;
  data_t m = a->effective_mass;
  // F = (2pi T)/(pitch)
  data_t f = M_PI * a->torque / a->pitch - a->gravity * a->mass * a->pitch;
  a->time = t;
  a->position = a->position + a->speed * dt;
  a->speed = a->speed * (1 - a->friction / m * dt) + f * dt / m;
  if (a->position < 0 || a->position > a->length) {
    a->speed = 0;
    a->position = a->position < 0 ? 0 : a->length;
    a->err_d = a->err_i = 0.0;
  }
}

static void *integrate(void *ud) {
  axis_t *axis = (axis_t *)ud;
  struct timeval tv;
  data_t time;
  while (axis->stop == 0) {
    gettimeofday(&tv, NULL);
    time = (tv.tv_sec - axis->t0) + ((data_t)tv.tv_usec) / 1E6;
    axis_forward_integrate(axis, time);
    usleep(axis->integration_dt);
  }
  fprintf(stderr, "Stopping integrator for axis %s\n", axis->name);
  axis->stop = 0;
  return NULL;
}

void axis_run(axis_t *axis) {
  pthread_attr_t attr;
  if (axis->running) {
    return;
  }
  axis->err_d = axis->err_i = 0.0;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  pthread_create(&axis->thread, &attr, integrate, (void *)axis);
  axis->running = 1;
}

void axis_stop(axis_t *axis) {
  axis->stop = 1;
  axis->running = 0;
}

//   __  __       _
//  |  \/  | __ _(_)_ __
//  | |\/| |/ _` | | '_ \ 
//  | |  | | (_| | | | | |
//  |_|  |_|\__,_|_|_| |_|

#ifdef AXIS_MAIN

int main(int argc, char const **argv) {
  // data_t dt = 0.005;
  // data_t time = 0;
  axis_t *ax = axis_new("machine.ini", "X");
  axis_t *ay = axis_new("machine.ini", "Y");
  axis_t *az = axis_new("machine.ini", "Z");
  axis_t *a;
  if (argc != 3) {
    eprintf("Usage: %s <seconds> <X|Y|Z>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  switch (toupper(argv[2][0]))
  {
  case 'X':
    a = ax;
    break;
  case 'Y':
    a = ay;
    break;
  case 'Z':
    a = az;
    break;
  default:
    eprintf("Wrong axis %c\n", argv[2][0]);
    exit(EXIT_FAILURE);
    break;
  }

  if (!ax || !ay || ! az) {
    exit(EXIT_FAILURE);
  }

  axis_set_torque(ax, 0);
  axis_set_torque(ay, 0);
  axis_set_torque(az, 0);
  axis_link(ay, az);
  axis_link(ax, ay);

  axis_reset(ax, 0);
  axis_reset(ay, 0);
  axis_reset(az, 0);

  printf("t q x v p i d\n");

  axis_run(ax);
  axis_run(ay);
  axis_run(az);

  fprintf(stderr, "axis %s: p %.3f i %.3f d %.3f\n", a->name, a->p, a->i, a->d);
  while (a->time < atof(argv[1])) {
    if (a->time < 1) {
      axis_set_setpoint(a, 0);
    } else {
      axis_set_setpoint(a, 0.5);
    }
    axis_pid(a);
    printf("%f %f %f %f %f %f %f\n", axis_time(a), axis_torque(a), axis_position(a),
           axis_speed(a), a->setpoint - a->position, a->err_i, a->err_d );
    usleep(10000);
  }
  axis_stop(ax);
  axis_stop(ay);
  axis_stop(az);
  usleep(500000);
  axis_free(ax);
  axis_free(ay);
  axis_free(az);
  return 0;
}

#endif