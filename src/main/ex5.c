//   _____                   _            ____  
//  | ____|_  _____ _ __ ___(_)___  ___  | ___| 
//  |  _| \ \/ / _ \ '__/ __| / __|/ _ \ |___ \ 
//  | |___ >  <  __/ | | (__| \__ \  __/  ___) |
//  |_____/_/\_\___|_|  \___|_|___/\___| |____/ 
// Calculate length and duration of a G-code program

#include "../defines.h"
#include "../program.h"
#include "../block.h"
#include "../machine.h"
#include <stdio.h>

int main(int argc, const char **argv) {
  data_t dt = 0;                   // block duration
  data_t duration = 0, length = 0; // total duration and length
  machine_t *machine = machine_new(argv[1]);
  program_t *prog = program_new(argv[2]);
  data_t fmax = machine_fmax(machine);
  block_t *block = NULL;

  // parse and print the program
  program_parse(prog, machine);
  program_print(prog, stderr);

  // loop over all the blocks
  while ((block = program_next(prog)) != NULL) {
    // block duration (estimate if block is rapid)
    dt = block_type(block) == RAPID ? block_length(block) / fmax : block_dt(block);
    // increment length and duration
    length += block_length(block);
    duration += dt;
    // print values for current block
    if (block_type(block) == RAPID) {
      printf(RED"N%03zu, length: %7.3f mm, duration %7.3f s\n"CRESET, block_n(block), block_length(block), dt);
    } else {
      printf(BLU"N%03zu, length: %7.3f mm, duration %7.3f s\n"CRESET, block_n(block), block_length(block), dt);
    }
  }

  // print overall length and duration
  printf(BGRN"TOT: length: %7.3f mm, duration: %7.3f s\n"CRESET, length, duration);

  // free resources
  program_free(prog);
  machine_free(machine);
  return EXIT_SUCCESS;
}