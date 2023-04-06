//   ____  _            _
//  | __ )| | ___   ___| | __
//  |  _ \| |/ _ \ / __| |/ /
//  | |_) | | (_) | (__|   <
//  |____/|_|\___/ \___|_|\_\

#include "defines.h"
#include "block.h"

//   ____            _                 _   _
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/

// Velocity profile data
typedef struct {
  data_t a, d;            // acceleration and deceleration
  data_t f, l;            // nominal feedrate and length
  data_t fs, fe;          // initial and final feedrate
  data_t dt_1, dt_m, dt2; // trapezoidal profile times
  data_t dt;              // total block duration
} block_profile_t;

// Object struct (opaque)
typedef struct block {
  char *line;            // G-code string
  block_type_t type;     // block type
  size_t n;              // block number
  size_t tool;           // tool number
  data_t feedrate;       // feedrate in mm/min
  data_t arc_feedrate;   // actual nominal feedrate along an arc motion
  data_t spindle;        // spindle rotational speed in RPM
  point_t *target;       // final coordinate of this block
  point_t *delta;        // projections
  point_t *center;       // arc center coordinates
  data_t length;         // segment of arc length
  data_t i, j, r;        // arc parameters
  data_t theta0, dtheta; // initial and arc angles
  data_t acc;            // actual acceleration
  block_profile_t *prof; // block velocity profile data
  struct block *prev;
  struct block *next;
} block_t;

// STATIC FUNCTIONS
static point_t *point_zero(block_t *b);

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// LIFECYCLE
block_t *block_new(char const *line, block_t *prev) {
  assert(line);
  // allocate memory
  block_t *b = malloc(sizeof(block_t));
  if (!b) {
    eprintf("Could not allocate memory for a block\n");
    return NULL;
  }

  if (prev) { // copy the memory from the previous block
    memcpy(b, prev, sizeof(block_t));
    b->prev = prev;
    prev->next = b;
  } else { // this is the first block: set everything to 0
    memset(b, 0, sizeof(block_t));
  }

  // in any casem all non-modal parameters are set to 0
  b->i = b->j = b->r = 0;
  b->length = 0;
  b->type = NO_MOTION;
  b->acc = 0;

  // fields to be calculated
  b->prof = malloc(sizeof(block_profile_t));
  if (!b->prof) {
    eprintf("Could not allocate memory for block profile\n");
    block_free(b);
    return NULL;
  }
  b->target = point_new();
  b->delta = point_new();
  b->center = point_new();
  if (!b->center || !b->delta || !b->target) {
    eprintf("Could not allocate points in a new block\n");
    block_free(b);
    return NULL;
  }
  b->line = strdup(line);
  if (!b->line) {
    eprintf("Could not allocate memory for block line\n");
    block_free(b);
    return NULL;
  }

  return b;
}

void block_free(block_t *b) {
  assert(b);
  if (b->line)
    free(b->line);
  if (b->prof)
    free(b->prof);
  if (b->target)
    point_free(b->target);
  if (b->delta)
    point_free(b->delta);
  if (b->center)
    point_free(b->center);
  free(b);
}

void block_print(block_t *b, FILE *out) {
  assert(b && out);
  char *start, *end;
  point_t *p0 = point_zero(b);
  point_inspect(p0, &start);
  point_inspect(b->target, &end);
  fprintf(out, "%03lu %s->%s F%7.1f S%7.1f T%2lu (G%02d)\n", b->n, start, end,
          b->feedrate, b->spindle, b->tool, b->type);
  free(start);
  free(end);
}

// ACCESSORS (all getters)

#define block_getter(typ, par, name)                                           \
  typ block_##name(block_t const *b) {                                         \
    assert(b);                                                                 \
    return b->par;                                                             \
  }

block_getter(data_t, length, length);
block_getter(data_t, dtheta, dtheta);
block_getter(data_t, prof->dt, dd);
block_getter(block_type_t, type, type);
block_getter(char *, line, line);
block_getter(size_t, n, n);
block_getter(data_t, r, r);
block_getter(point_t *, center, center);
block_getter(point_t *, target, target);
block_getter(block_t *, next, next);

// METHODS

int block_parse(block_t *b) { return 0; }

data_t block_lambda(block_t *b, data_t time, data_t *v) { return 0; }

point_t *block_interpolate(block_t *b, data_t lambda) { return NULL; }

//   ____  _        _   _         __                  _   _
//  / ___|| |_ __ _| |_(_) ___   / _|_   _ _ __   ___| |_(_) ___  _ __  ___
//  \___ \| __/ _` | __| |/ __| | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//   ___) | || (_| | |_| | (__  |  _| |_| | | | | (__| |_| | (_) | | | \__ \
//  |____/ \__\__,_|\__|_|\___| |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

static point_t *point_zero(block_t *b) {
  point_t *p = point_new();

  return p;
}