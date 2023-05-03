#include "../defines.h"
#include "../program.h"
#include "../block.h"
#include "../point.h"
#include "../machine.h"

int main(int argc, char const *argv[]) {
  machine_t *m = NULL;
  program_t *p = NULL;
  block_t *curr_b = NULL;
  data_t t, tt = 0, tq, lambda, v, dt;
  point_t *pos = NULL;
  
  if (argc != 3) {
    eprintf("I need exactly two arguments: g-code filename and INI filename\n");
    exit(EXIT_FAILURE);
  }
  m = machine_new(argv[2]);
  if (!m) {
    eprintf("Error in INI file\n");
    exit(EXIT_FAILURE);
  }
  p = program_new(argv[1]);
  if (!p) {
    eprintf("Could not create program\n");
    exit(EXIT_FAILURE);
  }
  if (program_parse(p, m) < 0) {
    eprintf("Could not parse program\n");
    exit(EXIT_FAILURE);
  }
  program_print(p, stderr);

  tq = machine_tq(m);
  // ------ Run program ------ //
  program_reset(p);
  printf("# N t tt lambda s v X Y Z\n");
  while ((curr_b = program_next(p))) {
    if (block_type(curr_b) == RAPID)
      continue;
    dt = block_dt(curr_b);
    for (t = 0; t <= dt + tq/10.0; t += tq, tt += tq) {
      lambda = block_lambda(curr_b, t, &v);
      pos = block_interpolate(curr_b, lambda);
      if (!pos) 
        continue;
      printf("%lu %.3f %.3f %.6f %.3f %.3f %.3f %.3f %.3f\n", block_n(curr_b), t, tt, lambda, lambda * block_length(curr_b), v, point_x(pos), point_y(pos), point_z(pos));
    }
  }
  // ------------------------- //

  program_free(p);
  machine_free(m);
  return 0;
}