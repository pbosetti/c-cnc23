//      _          _
//     / \   __  _(_)___
//    / _ \  \ \/ / / __|
//   / ___ \  >  <| \__ \
//  /_/   \_\/_/\_\_|___/
// Linear axis dynamic simulator

#ifndef AXIS_H
#define AXIS_H
#include "defines.h"

typedef struct axis axis_t;

// Lifecycle
axis_t *axis_new(char const *ini_path, char const *name);
void axis_free(axis_t *axis);
void axis_link(axis_t *master, axis_t *slave);

// Accessors
char const *axis_name(axis_t const *b);
data_t axis_position(axis_t const *b);
data_t axis_speed(axis_t const *b);
data_t axis_time(axis_t const *b);
data_t axis_setpoint(axis_t const *b);
data_t axis_length(axis_t const *b);
data_t axis_torque(axis_t const *b);
data_t axis_max_power(axis_t const *b);
axis_t *axis_linked(axis_t const *b);
int axis_running(axis_t const *b);

void axis_set_torque(axis_t *b, data_t value);
void axis_set_setpoint(axis_t *b, data_t value);
void axis_set_i(axis_t *b, data_t value);

// Dynamics
void axis_reset(axis_t *axis, data_t position);
void axis_pid(axis_t *axis);
void axis_forward_integrate(axis_t *axis, data_t time);
void axis_run(axis_t *axis);
void axis_stop(axis_t *axis);

#endif // AXIS_H