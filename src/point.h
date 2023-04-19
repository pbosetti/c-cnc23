//   ____       _       _
//  |  _ \ ___ (_)_ __ | |_
//  | |_) / _ \| | '_ \| __|
//  |  __/ (_) | | | | | |_
//  |_|   \___/|_|_| |_|\__|
// Point class

#ifndef POINT_H
#define POINT_H

#include "defines.h"

//   ____            _                 _   _
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/

// Point object struct (Opaque object!)
typedef struct point point_t;

// Bitmask values
#define X_SET '\1'
#define Y_SET '\2'
#define Z_SET '\4'
#define XYZ_SET '\7'

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// LIFECYCLE (instance creation/destruction) ===================================
point_t *point_new();
void point_free(point_t *p);
void point_inspect(point_t const *p, char **desc);

// ACCESSORS (getting/setting object fields) ===================================
// setters
void point_set_x(point_t *p, data_t x);
void point_set_y(point_t *p, data_t y);
void point_set_z(point_t *p, data_t z);
void point_set_xyz(point_t *p, data_t x, data_t y, data_t z);
// getters
data_t point_x(point_t const *p);
data_t point_y(point_t const *p);
data_t point_z(point_t const *p);

// METHODS (Functions that operate on an object) ===============================
data_t point_dist(point_t const *from, point_t const *to);
void point_delta(point_t const *from, point_t const *to, point_t *delta);
void point_modal(point_t const *from, point_t *to);

#endif // POINT_H