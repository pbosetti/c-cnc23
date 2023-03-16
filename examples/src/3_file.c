//   _____ _ _           
//  |  ___(_) | ___  ___ 
//  | |_  | | |/ _ \/ __|
//  |  _| | | |  __/\__ \
//  |_|   |_|_|\___||___/
                      
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// preprocessor constant
#define MAX_LINE_LENGTH 1024

// enum: lists the possible return values
// note: enum values are internally stored as ints. Typically, the first value
// is 0, the second 1 and so on, BUT THIS IT NOT GRANTED (different compilers
// can behave differently). If the numeric value is important (e.g. because you
// plan to transmit it via network), then it is safer to force its values. This
// can be done by explicitly give the value of 0 to the first element.
enum errors {
  ERROR_SUCCESS=0,
  ERROR_ARGS,
  ERROR_READ,
  ERROR_WRITE,
  ERROR_OPEN
};
// for brevity, we can define a custom type for the "enum errors":
typedef enum errors error_t;

// same result in a single step using an anonymous enum:
// typedef enum {
//   ERROR_SUCCESS=0,
//   ERROR_ARGS,
//   ERROR_READ,
//   ERROR_WRITE,
//   ERROR_OPEN
// } error_t;


int main(int argc, const char *argv[]) {
  error_t result = ERROR_SUCCESS;
  const char *filename;

  if (argc != 2) {
    fprintf(stderr, "I need 1 argument!\n");
    result = ERROR_ARGS;
    goto end;
  }

  filename = argv[1];
  fprintf(stdout, "My filename is: %s\n", filename);

  // a group of expressions can be surrounded by {} this creates a local scope:
  // variables declared herein are not visible outside the block
  {
    FILE *f = fopen(filename, "w");
    if (!f) {
      fprintf(stderr, "I cannot open the file: %s\n", filename);
      result = ERROR_OPEN;
      goto end;
    }

    fprintf(f, "I'm a line\n");
    fprintf(f, "I'm another line\n");
    fprintf(f, "I'm a number: %8.3f\n", 20.0f);

    fclose(f);
  }

  // another block with its local scope
  {
    char buffer[MAX_LINE_LENGTH];
    FILE *f = fopen(filename, "r"); // Not the same f as above!
    if (!f) {
      fprintf(stderr, "I cannot open the file: %s\n", filename);
      result = ERROR_OPEN;
      goto end;
    }

    while(fgets(buffer, MAX_LINE_LENGTH, f)) {
      fprintf(stdout, "%s", buffer);
    }

    fclose(f);
  }

  fprintf(stderr, "I'm an error!\n");

end:
  return ERROR_SUCCESS;
}