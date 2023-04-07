//   ____  _            _
//  | __ )| | ___   ___| | __
//  |  _ \| |/ _ \ / __| |/ /
//  | |_) | | (_) | (__|   <
//  |____/|_|\___/ \___|_|\_\
//
#include "block.h"
#include "defines.h"
#include <ctype.h> // toupper()
#include <math.h>
#include <sys/param.h> // MIN()

//   ____            _                 _   _
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/
//
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
  machine_t *machine;    // machine object
  struct block *prev;
  struct block *next;
} block_t;

// STATIC FUNCTIONS
static point_t *start_point(block_t *b);
static int block_arc(block_t *b);
static void block_compute(block_t *b);
static int block_set_fields(block_t *b, char cmd, char *arg);

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
//
// LIFECYCLE
block_t *block_new(char const *line, block_t *prev, machine_t *machine) {
  assert(line);
  // allocate memory
  block_t *b = malloc(sizeof(block_t));
  if (!b) {
    eprintf("Could not allocate memory for a block\n");
    goto fail;
  }

  if (prev) { // copy the memory from the previous block
    memcpy(b, prev, sizeof(block_t));
    b->prev = prev;
    prev->next = b;
  } else { // this is the first block: set everything to 0
    memset(b, 0, sizeof(block_t));
  }

  // in any case all non-modal parameters are set to 0
  b->i = b->j = b->r = 0;
  b->length = 0;
  b->type = NO_MOTION;

  // machine parameters
  b->machine = machine;
  b->acc = machine_A(machine);

  // fields to be calculated
  b->prof = malloc(sizeof(block_profile_t));
  if (!b->prof) {
    eprintf("Could not allocate memory for block profile\n");
    goto fail;
  }
  b->target = point_new();
  b->delta = point_new();
  b->center = point_new();
  if (!b->center || !b->delta || !b->target) {
    eprintf("Could not allocate points in a new block\n");
    goto fail;
  }
  b->line = strdup(line);
  if (!b->line) {
    eprintf("Could not allocate memory for block line\n");
    goto fail;
  }
  return b;
fail:
  if (b)
    block_free(b);
  return NULL;
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
  char *start = NULL, *end = NULL;
  point_inspect(start_point(b), &start);
  point_inspect(b->target, &end);
  fprintf(out, "%03lu %s->%s F%7.1f S%7.1f T%02lu (G%02d)\n", b->n, start, end,
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

int block_parse(block_t *b) {
  assert(b);
  char *word, *line, *tofree;
  point_t *p0;
  int rv = 0;

  tofree = line = strdup(b->line);
  if (!line) {
    eprintf("Could not allocate momory for tokenizing line\n");
    return -1;
  }
  // Tokenizing loop
  while ((word = strsep(&line, " ")) != NULL) {
    // word[0] is the command
    // word+1 is the pointer to the argument as a string
    rv += block_set_fields(b, toupper(word[0]), word + 1);
  }
  free(tofree);

  // inherit modal fields from the previous block
  p0 = start_point(b);
  point_modal(p0, b->target);
  point_delta(p0, b->target, b->delta);
  b->length = point_dist(p0, b->target);

  // deal with motion blocks
  switch (b->type) {
  case LINE:
    // calculate feed profile
    b->acc = machine_A(b->machine);
    b->arc_feedrate = b->feedrate;
    block_compute(b);
    break;
  case ARC_CW:
  case ARC_CCW:
    // calculate arc coordinates
    if (block_arc(b)) {
      wprintf("Could not calculate acr coordinates\n");
      rv++;
      break;
    }
    // set corrected feedrate and acceleration
    // centripetal acc = f^2/r, must be <= A
    // INI file gives A in mm/s^2, feedrate is given in mm/min
    // We divide by two because, in the critical condition where we have
    // the maximum feedrate, in the following equation for calculating the
    // acceleration, it goes to 0. In fact, if we accept the centripetal
    // acceleration to reach the maximum acceleration, then the tangential
    // acceleration would go to 0.
    // A more elegant solution would be to calculate a minimum time soltion
    // for the whole arc, but it is outside the scope.
    b->arc_feedrate =
        MIN(b->feedrate, sqrt(machine_A(b->machine) / 2.0 * b->r) * 60);
    // tangential acceleration: when composed with centripetal one, total
    // acceleration must be <= A
    // a^2 <= A^2 + v^4/r^2
    b->acc = sqrt(pow(machine_A(b->machine), 2) -
                  pow(b->arc_feedrate / 60, 4) / pow(b->r, 2));
    // deal with complex result
    if (isnan(b->acc)) {
      wprintf("Cannot compute arc: insufficient acceleration\n");
      rv++;
    }
    // calculate feed profile
    block_compute(b);
    break;
  default:
    break;
  }
  // return number of parsing errors
  return rv;
}

data_t block_lambda(block_t *b, data_t time, data_t *v) { return 0; }

point_t *block_interpolate(block_t *b, data_t lambda) { return NULL; }

//   ____  _        _   _         __                  _   _
//  / ___|| |_ __ _| |_(_) ___   / _|_   _ _ __   ___| |_(_) ___  _ __  ___
//  \___ \| __/ _` | __| |/ __| | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//   ___) | || (_| | |_| | (__  |  _| |_| | | | | (__| |_| | (_) | | | \__ \
//  |____/ \__\__,_|\__|_|\___| |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// Return a reliable previous point, i.e. machine zero if this is the first
// block
static point_t *start_point(block_t *b) {
  assert(b);
  return b->prev ? b->prev->target : machine_zero(b->machine);
}

static int block_set_fields(block_t *b, char cmd, char *arg) {
  assert(b && arg);
  switch (cmd)
  {
  case 'N':
    b->n = atol(arg);
    break;
  case 'G':
    b->type = (block_type_t)atoi(arg);
    break;
  case 'X':
    point_set_x(b->target, atof(arg));
    break;
  case 'Y':
    point_set_y(b->target, atof(arg));
    break;
  case 'Z':
    point_set_z(b->target, atof(arg));
    break;
  case 'I': 
    b->i = atof(arg);
    break;
  case 'J':
    b->j = atof(arg);
    break;
  case 'R':
    b->r = atof(arg);
    break;
  case 'F':
    b->feedrate = atof(arg);
    break;
  case 'S':
    b->spindle = atof(arg);
    break; 
  case 'T':
    b->tool = atol(arg);   
    break;
  default:
    fprintf(stderr, "ERROR: Usupported G-code command %c%s\n", cmd, arg);
    return 1;
    break;
  }
  // cannot have R and IJ on the same block
  if (b->r && (b->i || b->j)) {
    fprintf(stderr, "ERROR: Cannot mix R and IJ\n");
    return 1;
  }
  return 0;
}

static int block_arc(block_t *b) { return 0; }

static void block_compute(block_t *b) {}

//   __  __       _
//  |  \/  | __ _(_)_ __
//  | |\/| |/ _` | | '_ \
//  | |  | | (_| | | | | |
//  |_|  |_|\__,_|_|_| |_|
//
#ifdef BLOCK_MAIN
int main(int argc, char const *argv[]) {
  block_t *b1 = NULL, *b2 = NULL, *b3 = NULL;
  machine_t *cfg = machine_new(argv[1]);
  if (!cfg) exit(EXIT_FAILURE);

  b1 = block_new("N10 G00 X90 Y90 Z100 t3", NULL, cfg);
  block_parse(b1);
  b2 = block_new("N20 G01 Y100 X100 F1000 S2000", b1, cfg);
  block_parse(b2);
  b3 = block_new("N30 G01 Y200", b2, cfg);
  block_parse(b3);

  block_print(b1, stdout);
  block_print(b2, stdout);
  block_print(b3, stdout);

  block_free(b1);
  block_free(b2);
  block_free(b3);
  return 0;
}
#endif