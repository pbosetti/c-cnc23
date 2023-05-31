//   __  __            _     _
//  |  \/  | __ _  ___| |__ (_)_ __   ___
//  | |\/| |/ _` |/ __| '_ \| | '_ \ / _ \
//  | |  | | (_| | (__| | | | | | | |  __/
//  |_|  |_|\__,_|\___|_| |_|_|_| |_|\___|
// Machine class
#ifndef MACHINE_H
#define MACHINE_H

#include "defines.h"
#include "point.h"
#include <mosquitto.h>

//   _____
//  |_   _|   _ _ __   ___  ___
//    | || | | | '_ \ / _ \/ __|
//    | || |_| | |_) |  __/\__ \
//    |_| \__, | .__/ \___||___/
//        |___/|_|

// Opaque struct as object
typedef struct machine machine_t;

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// Lifecycle ===================================================================
machine_t *machine_new(char const *cfg_path);
void machine_free(machine_t *m);

// Accessors ===================================================================
data_t machine_A(machine_t const *m);
data_t machine_tq(machine_t const *m);
data_t machine_max_error(machine_t const *m);
data_t machine_error(const machine_t *m);
data_t machine_fmax(const machine_t *m);
data_t machine_rt_pacing(machine_t const *m);
point_t *machine_zero(machine_t const *m);
point_t *machine_setpoint(machine_t const *m);
point_t *machine_position(machine_t const *m);

// Methods =====================================================================
void machine_print_params(machine_t const *m);

// MQTT-related
typedef void (* machine_on_message)(struct mosquitto *mqt, void *userdata, struct mosquitto_message const *msg);

int machine_connect(machine_t *m, machine_on_message callback);

int machine_sync(machine_t *m, int rapid);

int machine_listen_start(machine_t *m);

int machine_listen_stop(machine_t *m);

void machine_disconnect(machine_t *m);

#endif // MACHINE_H