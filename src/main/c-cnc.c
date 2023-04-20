
#include "../defines.h"
#include "../program.h"
#include "../machine.h"

int main(int argc, char const *argv[]) {
  point_t *sp = NULL;
  block_t *b = NULL;
  program_t *p = NULL;
  data_t t, tt, tq, lambda, f;
  machine_t *machine = machine_new("settings.ini");
  if (!machine) {
    eprintf("Error creating machine instance\n");
    exit(EXIT_FAILURE);
  }
  tq = machine_tq(machine);

  p = program_new(argv[1]);
  if (!p) {
    eprintf("Could not create program, exiting.\n");
    exit(EXIT_FAILURE);
  }
  if (program_parse(p, machine) == EXIT_FAILURE) {
    eprintf("Could not parse program in %s, exiting.\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  program_print(p, stderr);
  
  machine_connect(machine, NULL);

  // main loop
  printf("n,t,tt,lambda,s,f,x,y,z\n");
  tt = 0;
  while ((b = program_next(p))) {
    if (block_type(b) == RAPID || block_type(b) > ARC_CCW) {
      continue;
    }
    eprintf("Interpolating the block %s\n", block_line(b));
    // interpolation loop
    // careful: we check t <= block_dt(b) + tq/2.0 for double values are
    // never exact, and we may have that adding many tq carries over a small
    // error that accumuates and may result in n*tb being greater than Dt
    // (if so, we would miss the last step)
    for (t = 0; t <= block_dt(b) + tq/2.0; t += tq, tt += tq) {
      lambda = block_lambda(b, t, &f);
      sp = block_interpolate(b, lambda);
      if (!sp) continue;
      printf("%lu,%f,%f,%f,%f,%f,%f,%f,%f\n", block_n(b), t, tt,
        lambda, lambda * block_length(b), f,
        point_x(sp), point_y(sp), point_z(sp));
      machine_sync(machine, 0);
      // wait_next(5e6);
    }
  }


  machine_free(machine);
  program_free(p);
  return 0;
}