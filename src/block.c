//   ____  _            _
//  | __ )| | ___   ___| | __
//  |  _ \| |/ _ \ / __| |/ /
//  | |_) | | (_) | (__|   <
//  |____/|_|\___/ \___|_|\_\
//
#include "block.h"
#include "defines.h"
#include "machine.h"
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
  data_t a, d;             // acceleration and deceleration
  data_t f, l;             // nominal feedrate and length
  data_t fs, fe;           // initial and final feedrate
  data_t dt_1, dt_m, dt_2; // trapezoidal profile times
  data_t dt;               // total block duration
} block_profile_t;

// Object struct (opaque)
typedef struct block {
  char *line;               // G-code string
  block_type_t type;        // block type
  size_t n;                 // block number
  size_t tool;              // tool number
  data_t feedrate;          // feedrate in mm/min
  data_t arc_feedrate;      // actual nominal feedrate along an arc motion
  data_t spindle;           // spindle rotational speed in RPM
  point_t *target;          // final coordinate of this block
  point_t *delta;           // projections
  point_t *center;          // arc center coordinates
  data_t length;            // segment of arc length
  data_t i, j, r;           // arc parameters
  data_t theta0, dtheta;    // initial and arc angles
  data_t acc;               // actual acceleration
  block_profile_t *prof;    // block velocity profile data
  machine_t const *machine; // machine object (holding config data)
  struct block *prev;
  struct block *next;
} block_t;

// STATIC FUNCTIONS
static point_t *start_point(block_t *b);
static int block_set_fields(block_t *b, char cmd, char *argv);
static void block_compute(block_t *b);
static int block_arc(block_t *b);

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
//
// LIFECYCLE ===================================================================
block_t *block_new(char const *line, block_t *prev, machine_t const *machine) {
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
  b->acc = machine_A(b->machine);

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
  point_t *p0 = start_point(b);
  point_inspect(p0, &start);
  point_inspect(b->target, &end);
  fprintf(out, "%03lu %s->%s F%7.1f S%7.1f T%2lu (G%02d)\n", b->n, start, end,
          b->feedrate, b->spindle, b->tool, b->type);
  free(start);
  free(end);
}

// ACCESSORS (all getters) =====================================================

#define block_getter(typ, par, name)                                           \
  typ block_##name(block_t const *b) {                                         \
    assert(b);                                                                 \
    return b->par;                                                             \
  }

block_getter(data_t, length, length);
block_getter(data_t, dtheta, dtheta);
block_getter(data_t, prof->dt, dt);
block_getter(block_type_t, type, type);
block_getter(char *, line, line);
block_getter(size_t, n, n);
block_getter(data_t, r, r);
block_getter(point_t *, center, center);
block_getter(point_t *, target, target);
block_getter(block_t *, next, next);


// METHODS =====================================================================

// we have a G-code line like "N10 G01 X0 Y100  z210.5 F1000 S5000"
int block_parse(block_t *b) {
  assert(b);
  point_t *p0 = NULL;
  int rv = 0;
  char *word, *line, *tofree;

  tofree = line = strdup(b->line);
  if (!line) {
    eprintf("Could not allocate memory for line string\n");
    return -1;
  }
  // tokenization
  while ((word = strsep(&line, " ")) != NULL) {
    // word[0] is the first character (the command)
    // word + 1 is the string beginning after the forst character
    rv += block_set_fields(b, toupper(word[0]), word + 1);
  }
  free(tofree);

  // inherit coordinates from previous point
  p0 = start_point(b);
  point_modal(p0, b->target);
  point_delta(p0, b->target, b->delta);
  b->length = point_dist(p0, b->target);

  // deal with motion blocks
  switch (b->type) {
  case LINE:
    b->acc = machine_A(b->machine);
    b->arc_feedrate = b->feedrate;
    block_compute(b);
    break;

  case ARC_CW:
  case ARC_CCW:
    if (block_arc(b)) {
      wprintf("Could not calculate arc coordinates\n");
      rv++;
      break;
    }
    // Set corrected feedrate and acceleration
    // Centripetal acc = f^2/r, must be <= A
    // We divide by two because with a tangential component equal to A,
    // any non-null feedrate is providing a total acceleration in excess of A.
    b->acc = machine_A(b->machine) / 2.0;
    // Then: a_m^2/4 + (a_m^4 t^4) / (16 r^2) = A^2
    // solve for t: t^4 = 12 r^2 / a_m^2 (t at max total acceleration)
    // calculate feedrate at t: f = a_m/2 t = (3/4 a_m^2 r^2)^0.25
    // INI file gives A in mm/s^2, feedrate is given in mm/min.
    // A more elegant solution would be to calculate a minimum time soltion
    // for the whole arc, but it is outside the scope.
    b->arc_feedrate = MIN(
        b->feedrate,
        pow(3.0 / 4.0 * pow(machine_A(b->machine), 2) * pow(b->r, 2), 0.25) *
            60);
    block_compute(b);
    break;
  default:
    break;
  }
  return rv;
}

data_t block_lambda(block_t *b, data_t t, data_t *v) {
  assert(b);
  data_t r;
  data_t dt_1 = b->prof->dt_1;
  data_t dt_2 = b->prof->dt_2;
  data_t dt_m = b->prof->dt_m;
  data_t a = b->prof->a;
  data_t d = b->prof->d;
  data_t f = b->prof->f;

  if (t < 0) {
    r = 0.0;
    *v = 0.0;
  } else if (t < dt_1) { // acceleration
    r = a * pow(t, 2) / 2.0;
    *v = a * t;
  } else if (t < dt_1 + dt_m) { // maintenance
    r = f * (dt_1 / 2.0 + (t - dt_1));
    *v = f;
  } else if (t < dt_1 + dt_m + dt_2) { // deceleration
    data_t t_2 = dt_1 + dt_m;
    r = f * dt_1 / 2.0 + f * (dt_m + t - t_2) +
        d / 2.0 * (pow(t, 2) + pow(t_2, 2)) - d * t * t_2;
    *v = f + d * (t - t_2);
  } else {
    r = b->prof->l;
    *v = 0.0;
  }

  r /= b->prof->l;
  *v *= 60; // convert to mm/min
  return r;
}

point_t *block_interpolate(block_t *b, data_t lambda) {
  assert(b);
  point_t *result = machine_setpoint(b->machine);
  point_t *p0 = start_point(b);

  // Parametric equations of segment:
  // x(t) = x(0) + d_x * lambda
  // y(t) = y(0) + d_y * lambda
  if (b->type == LINE) {
    point_set_x(result, point_x(p0) + point_x(b->delta) * lambda);
    point_set_y(result, point_y(p0) + point_y(b->delta) * lambda);
  }
  // paremetric equations of arc:
  // x(t) = x_c + R cos(theta_0 + dtheta * lambda)
  // y(t) = y_c + R sin(theta_0 + dtheta * lambda)
  else if (b->type == ARC_CW || b->type == ARC_CCW) {
    point_set_x(result, point_x(b->center) +
                            b->r * cos(b->theta0 + b->dtheta * lambda));
    point_set_y(result, point_y(b->center) +
                            b->r * sin(b->theta0 + b->dtheta * lambda));
  } else {
    wprintf("Unexpected block type\n");
    return NULL;
  }
  // Z is always linearly intepolated (arc becomes spiral)
  point_set_z(result, point_z(p0) + point_z(b->delta) * lambda);
  return result;
}

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
  switch (cmd) {
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
    if (strcmp(arg, "MAX") == 0) {
      b->feedrate = machine_fmax(b->machine);
    } else {
      b->feedrate = MIN(atof(arg), machine_fmax(b->machine));
    }
    break;
  case 'S':
    b->spindle = atof(arg);
    break;
  case 'T':
    b->tool = atol(arg);
    break;

  default:
    wprintf("Unsupported command %c\n", cmd);
    return 1;
    break;
  }
  // Both R and IJ are specified
  if (b->r && (b->i || b->j)) {
    wprintf("Cannot mix R and I,J\n");
    return 1;
  }
  return 0;
}

static data_t quantize(data_t t, data_t tq, data_t *dq) {
  data_t q;
  q = ((size_t)(t / tq) + 1) * tq;
  *dq = q - t;
  return q;
}

static void block_compute(block_t *b) {
  assert(b);
  data_t A, a, d;
  data_t dt, dt_1, dt_2, dt_m, dq;
  data_t f_m, l;

  A = b->acc;
  f_m = b->arc_feedrate / 60.0;
  l = b->length;
  dt_1 = f_m / A;
  dt_2 = dt_1;
  dt_m = l / f_m - (dt_1 + dt_2) / 2.0;
  if (dt_m > 0) { // trapezoidal profile
    dt = quantize(dt_1 + dt_m + dt_2, machine_tq(b->machine), &dq);
    dt_m += dq;
    f_m = (2 * l) / (dt_1 + dt_2 + 2 * dt_m);
  } else { // triangular profile (short block)
    dt_1 = dt_2 = sqrt(l / A);
    dt = quantize(dt_1 + dt_2, machine_tq(b->machine), &dq);
    dt_m = 0;
    dt_2 += dq;
    f_m = 2 * l / (dt_1 + dt_2);
  }
  a = f_m / dt_1;
  d = -(f_m / dt_2);
  // copy back values into block object
  b->prof->dt_1 = dt_1;
  b->prof->dt_2 = dt_2;
  b->prof->dt_m = dt_m;
  b->prof->a = a;
  b->prof->d = d;
  b->prof->f = f_m;
  b->prof->dt = dt;
  b->prof->l = l;
}

// Calculate the arc coordinates
// see slides pages 107-109
static int block_arc(block_t *b) {
  data_t x0, y0, z0, xc, yc, xf, yf, zf, r;
  point_t *p0 = start_point(b);
  x0 = point_x(p0);
  y0 = point_y(p0);
  z0 = point_z(p0);
  xf = point_x(b->target);
  yf = point_y(b->target);
  zf = point_z(b->target);

  if (b->r) { // if the radius is given
    data_t dx = point_x(b->delta);
    data_t dy = point_y(b->delta);
    r = b->r;
    data_t dxy2 = pow(dx, 2) + pow(dy, 2);
    data_t sq = sqrt(-pow(dy, 2) * dxy2 * (dxy2 - 4 * r * r));
    // signs table
    // sign(r) | CW(-1) | CCW(+1)
    // --------------------------
    //      -1 |     +  |    -
    //      +1 |     -  |    +
    int s = (r > 0) - (r < 0);
    s *= (b->type == ARC_CCW ? 1 : -1);
    xc = x0 + (dx - s * sq / dxy2) / 2.0;
    yc = y0 + dy / 2.0 + s * (dx * sq) / (2 * dy * dxy2);
  }
  else { // if I,J are given
    data_t r2;
    r = hypot(b->i, b->j);
    xc = x0 + b->i;
    yc = y0 + b->j;
    r2 = hypot(xf - xc, yf - yc);
    if (fabs(r - r2) > machine_error(b->machine)) {
      fprintf(stderr, "Arc endpoints mismatch error (%f)\n", r - r2);
      return 1;
    }
    b->r = r;
  }
  point_set_x(b->center, xc);
  point_set_y(b->center, yc);
  b->theta0 = atan2(y0 - yc, x0 - xc);
  b->dtheta = atan2(yf - yc, xf - xc) - b->theta0;
  // we need the net angle so we take the 2PI complement if negative
  if (b->dtheta <0) 
    b->dtheta = 2 * M_PI + b->dtheta;
  // if CW, take the negative complement
  if (b->type == ARC_CW)
    b->dtheta = -(2 * M_PI - b->dtheta);
  //
  b->length = hypot(zf - z0, b->dtheta * b->r);
  // from now on, it's safer to drop the sign of radius angle
  b->r = fabs(b->r);
  return 0;
}


//   _____         _                     _
//  |_   _|__  ___| |_   _ __ ___   __ _(_)_ __
//    | |/ _ \/ __| __| | '_ ` _ \ / _` | | '_ \
//    | |  __/\__ \ |_  | | | | | | (_| | | | | |
//    |_|\___||___/\__| |_| |_| |_|\__,_|_|_| |_|

#ifdef BLOCK_MAIN
int main(int argc, char const *argv[]) {
  machine_t *m = machine_new(argv[1]);
  block_t *b1 = NULL, *b2 = NULL, *b3 = NULL, *b4 = NULL;
  if (!m) {
    eprintf("Error creating machine\n");
    exit(EXIT_FAILURE);
  }

  b1 = block_new("N10 G01 X90 Y90 Z100 T3 F1000", NULL, m);
  block_parse(b1);
  b2 = block_new("N20 G01 y100 S2000", b1, m);
  block_parse(b2);
  b3 = block_new("N30 G01 Y200", b2, m);
  block_parse(b3);
  b4 = block_new("N40 G01 x0 y0 z0", b3, m);
  block_parse(b4);

  block_print(b1, stderr);
  block_print(b2, stderr);
  block_print(b3, stderr);
  block_print(b4, stderr);

  wprintf("Intepolation of block 20 (duration: %f)\n", block_dt(b2));
  {
    data_t t = 0, tq = machine_tq(m), dt = block_dt(b2);
    data_t lambda = 0, v = 0;
    printf("t lambda v x y z\n");
    for (t = 0; t - dt <= tq/10.0; t += tq) {
      lambda = block_lambda(b2, t, &v);
      block_interpolate(b2, lambda);
      printf("%f %f %f %.3f %.3f %.3f\n", t, lambda, v, 
        point_x(machine_setpoint(m)),
        point_y(machine_setpoint(m)),
        point_z(machine_setpoint(m)));
    }
  }

  block_free(b1);
  block_free(b2);
  block_free(b3);
  block_free(b4);
  machine_free(m);
  return 0;
}

#endif