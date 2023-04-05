//   ____       _       _   
//  |  _ \ ___ (_)_ __ | |_ 
//  | |_) / _ \| | '_ \| __|
//  |  __/ (_) | | | | | |_ 
//  |_|   \___/|_|_| |_|\__|
// Point class
#include "point.h"
#include <string.h>
#include <math.h>

//   _____                 _   _                 
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___ 
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
                                              
// LIFECYCLE (instance creation/destruction)

point_t *point_new() {
  point_t *p = malloc(sizeof(point_t));
  if (!p) {
    eprintf("Error allocating memory for a point\n");
    exit(EXIT_FAILURE);
  }
  memset(p, 0, sizeof(*p));
  return p;
}

void point_free(point_t *p) {
  assert(p);
  free(p);
}


// ACCESSORS (getting/setting object fields)

// Bitmask logics:
// xxxx xxxx -> initial value of bitmask
// 0000 0001 -> X_SET, value of '\1'
// ---------
// xxxx xxx1 -> result of a bitwise OR
void point_set_x(point_t *p, data_t x) {
  assert(p);
  p->x = x;
  p->s = p->s | X_SET;
}

// Bitmask logics:
// xxxx xxxx -> initial value of bitmask
// 0000 0010 -> X_SET, value of '\2'
// ---------
// xxxx xx1x -> result of a bitwise OR
void point_set_y(point_t *p, data_t y) {
  assert(p);
  p->y = y;
  p->s |= Y_SET;
}

// Bitmask logics:
// xxxx xxxx -> initial value of bitmask
// 0000 0100 -> X_SET, value of '\4'
// ---------
// xxxx x1xx -> result of a bitwise OR
void point_set_z(point_t *p, data_t z) {
  assert(p);
  p->z = z;
  // shortcut operators like += -=
  p->s |= Z_SET;
}

// Bitmask logics:
// xxxx xxxx -> initial value of bitmask
// 0000 0111 -> XYZ_SET, value of '\7'
// ---------
// xxxx x111 -> result of a bitwise OR
void point_set_xyz(point_t *p, data_t x, data_t y, data_t z) {
  assert(p);
  p->x = x;
  p->y = y;
  p->z = z;
  p->s = XYZ_SET;
}


// METHODS (Functuions that operate on an object)

data_t point_dist(point_t const *from, point_t const *to) {
  assert(from && to);
  return sqrt(
    pow(to->x - from->x, 2) +
    pow(to->y - from->y, 2) +
    pow(to->z - from->z, 2)
  );
}

void point_delta(point_t const *from, point_t const *to, point_t *delta) {
  assert(from && to);
  delta->x = to->x - from->x;
  delta->y = to->y - from->y;
  delta->z = to->z - from->z;
}


// N100 G01 X0 Y0 Z0
// N110 G01 X100
void point_modal(point_t const *from, point_t *to) {

}