//   ____       _       _
//  |  _ \ ___ (_)_ __ | |_
//  | |_) / _ \| | '_ \| __|
//  |  __/ (_) | | | | | |_
//  |_|   \___/|_|_| |_|\__|
// Point class
#include "point.h"
#include <math.h>
#include <string.h>

// Opaque oject, fields are only visible from within this file
typedef struct point {
  data_t x, y, z; // coordinates
  uint8_t s;      // bitmask
} point_t;

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// LIFECYCLE (instance creation/destruction) ===================================

point_t *point_new() {
  point_t *p = malloc(sizeof(point_t));
  if (!p) {
    eprintf("Error allocating memory for a point\n");
    return NULL;
  }
  memset(p, 0, sizeof(*p));
  return p;
}

void point_free(point_t *p) {
  assert(p);
  free(p);
}

#define FIELD_SIZE 8
#define FORMAT "[" RED "%s " GRN "%s " BLU "%s" CRESET "]"
void point_inspect(point_t const *p, char **desc) {
  assert(p);
  char str_x[FIELD_SIZE + 1], str_y[FIELD_SIZE + 1], str_z[FIELD_SIZE + 1];
  if (p->s & X_SET) {
    sprintf(str_x, "%*.3f", FIELD_SIZE, p->x);
  } else {
    sprintf(str_x, "%*s", FIELD_SIZE, "-");
  }
  if (p->s & Y_SET) {
    sprintf(str_y, "%*.3f", FIELD_SIZE, p->y);
  } else {
    sprintf(str_y, "%*s", FIELD_SIZE, "-");
  }
  if (p->s & Z_SET) {
    sprintf(str_z, "%*.3f", FIELD_SIZE, p->z);
  } else {
    sprintf(str_z, "%*s", FIELD_SIZE, "-");
  }
  if (*desc) {
    snprintf(*desc, strlen(*desc)+1, FORMAT, str_x, str_y, str_z);
  } else {
    if (asprintf(desc, FORMAT, str_x, str_y, str_z) == -1) {
      eprintf("Could not allocate memory for point description.\n");
      exit(EXIT_FAILURE);
    }
  }
}
#undef FIELD_SIZE
#undef FORMAT

// ACCESSORS (getting/setting object fields) ===================================

// This is the NON_DRY version of setters/getters definition: it is repetitive
// and error prone, so we keep it here for the sake of understanding, but it
// is actually disabled
#ifdef NOT_DRY
// Setters
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

// getters:
data_t point_x(point_t const *p) {
  assert(p);
  return p->x;
}
data_t point_y(point_t const *p) {
  assert(p);
  return p->y;
}
data_t point_z(point_t const *p) {
  assert(p);
  return p->z;
}
#else
// D.R.Y. Version, using a macro function for generating both accessors:
#define point_accessor(axis, bitmask)                                          \
  void point_set_##axis(point_t *p, data_t value) {                            \
    assert(p);                                                                 \
    p->axis = value;                                                           \
    p->s |= bitmask;                                                           \
  }                                                                            \
  data_t point_##axis(point_t const *p) {                                      \
    assert(p);                                                                 \
    return p->axis;                                                            \
  }

point_accessor(x, X_SET);
point_accessor(y, Y_SET);
point_accessor(z, Z_SET);
#endif

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

// METHODS (Functions that operate on an object) ===============================

data_t point_dist(point_t const *from, point_t const *to) {
  assert(from && to);
  return sqrt(pow(to->x - from->x, 2) + pow(to->y - from->y, 2) +
              pow(to->z - from->z, 2));
}

void point_delta(point_t const *from, point_t const *to, point_t *delta) {
  assert(from && to && delta);
  point_set_xyz(delta, to->x - from->x, to->y - from->y, to->z - from->z);
}

void point_modal(point_t const *from, point_t *to) {
  assert(from && to);
  if (!(to->s & X_SET) && (from->s & X_SET)) {
    point_set_x(to, from->x);
  }
  if (!(to->s & Y_SET) && (from->s & Y_SET)) {
    point_set_y(to, from->y);
  }
  if (!(to->s & Z_SET) && (from->s & Z_SET)) {
    point_set_z(to, from->z);
  }
}


//   _____         _   
//  |_   _|__  ___| |_ 
//    | |/ _ \/ __| __|
//    | |  __/\__ \ |_ 
//    |_|\___||___/\__|
                    
// This main function is conditionally enabled for testing purpose only.
#ifdef POINT_MAIN
int main(int argc, char const *argv[]) {
  point_t *p1, *p2, *p3;
  char *desc = NULL;
  // create three points:
  p1 = point_new();
  p2 = point_new();
  p3 = point_new();

  // set p1 to origin:
  point_set_xyz(p1, 0, 0, 0);
  // only set X and Y of second point:
  point_set_x(p2, 100);
  point_set_y(p2, 110);
  point_inspect(p2, &desc);
  printf("Initial p2:         %s\n", desc);
  // modal:
  point_modal(p1, p2);
  point_inspect(p2, &desc);
  printf("After modal p2:     %s\n", desc);
  // distance
  printf("Distance p1->p2:    %f\n", point_dist(p1, p2));
  // Delta:
  point_delta(p1, p2, p3);
  point_inspect(p3, &desc);
  printf("Projections p1->p2: %s\n", desc);
  // free the memory:
  point_free(p1);
  point_free(p2);
  point_free(p3);
  free(desc);

  return 0;
}
#endif