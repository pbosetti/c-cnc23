//   ____
//  |  _ \ _ __ ___   __ _ _ __ __ _ _ __ ___
//  | |_) | '__/ _ \ / _` | '__/ _` | '_ ` _ \ 
//  |  __/| | | (_) | (_| | | | (_| | | | | | |
//  |_|   |_|  \___/ \__, |_|  \__,_|_| |_| |_|
//                   |___/

#include "program.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//   _____
//  |_   _|   _ _ __   ___  ___
//    | || | | | '_ \ / _ \/ __|
//    | || |_| | |_) |  __/\__ \
//    |_| \__, | .__/ \___||___/
//        |___/|_|

// Object structure:
typedef struct program {
  char *filename;                  // path to the G-code program file
  FILE *file;                      // file pointer
  block_t *first, *current, *last; // relevant blocks in the linked list
  size_t n;                        // total number of blocks inthe program
} program_t;

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// Lifecycle ===================================================================
program_t *program_new(char const *filename) {
  program_t *p = malloc(sizeof(*p));
  // checks
  if (!p) {
    eprintf("Could not create program\n");
    return NULL;
  }
  if (!filename || strlen(filename) == 0) {
    eprintf("Improper or empty file name\n");
    free(p);
    return NULL;
  }
  // initialize fields
  p->filename = strdup(filename);
  p->first = NULL;
  p->last = NULL;
  p->current = NULL;
  p->n = 0;
  return p;
}

void program_free(program_t *p) {
  assert(p);
  block_t *b, *tmp;
  // free the linked list of blocks
  if (p->n > 0) {
    b = p->first;
    do {
      tmp = b;
      b = block_next(b);
      block_free(tmp);
    } while (b);
  }
  free(p->filename);
  free(p);
}

void program_print(program_t *p, FILE *output) {
  assert(p && output);
  block_t *b = p->first;
  // print each block from first to last
  do {
    block_print(b, output);
    b = block_next(b);
  } while (b);
}

// Accessors ===================================================================
#define program_getter(typ, par, name)                                         \
  typ program_##name(program_t const *p) {                                     \
    assert(p);                                                                 \
    return p->par;                                                             \
  }

program_getter(char *, filename, filename);
program_getter(block_t *, first, first);
program_getter(block_t *, current, current);
program_getter(block_t *, last, last);
program_getter(size_t, n, length);

// Processing ==================================================================

// Loop over each block and parse it
int program_parse(program_t *p, machine_t *machine) {
  assert(p && machine);
  char *line = NULL;
  ssize_t line_len = 0;
  size_t n = 0;
  block_t *b;
  p->n = 0;

  // open the g-code file
  p->file = fopen(p->filename, "r");
  if (!p->file) {
    eprintf("Cannot open the file %s\n", p->filename);
    return -1;
  }
  // make a loop: for each line in the file, create a new block with it
  p->n = 0;
  while ((line_len = getline(&line, &n, p->file)) >= 0) {
    // remove trailing newline \n replacing it with a string terminator \0
    if (line[line_len - 1] == '\n') {
      line[line_len - 1] = '\0';
    }
    // create a new block
    if (!(b = block_new(line, p->last, machine))) {
      eprintf("Error creating a block from line %s\n", line);
      return -1;
    }
    // parse the block
    if (block_parse(b)) {
      eprintf("Error parsing the block %s\n", line);
      return -1;
    }
    if (p->first == NULL) {
      p->first = b;
    }
    p->last = b;
    p->n++;
  }
  // cleanup
  fclose(p->file);
  free(line);
  program_reset(p);
  return p->n;
}

// load the next block
block_t *program_next(program_t *p) {
  assert(p);
  if (p->current == NULL) {
    p->current = p->first;
  } else {
    p->current = block_next(p->current);
  }
  return p->current;
}

void program_reset(program_t *p) {
  assert(p);
  p->current = NULL;
}

//   _____         _
//  |_   _|__  ___| |_
//    | |/ _ \/ __| __|
//    | |  __/\__ \ |_
//    |_|\___||___/\__|

#ifdef PROGRAM_MAIN
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

#endif