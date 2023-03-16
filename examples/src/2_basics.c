//   ____            _
//  | __ )  __ _ ___(_) ___ ___
//  |  _ \ / _` / __| |/ __/ __|
//  | |_) | (_| \__ \ | (__\__ \
//  |____/ \__,_|___/_|\___|___/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  // DECLARATIONS
  // better put them at the beginning of code block 
  double factor[2];
  unsigned int i;
  // declaration and initialization on the same line are OK too
  double result = 0; // default result code: 0 means "OK!"

  // CODE
  // check CLI arguments
  if (argc != 3) {
    fprintf(stderr, "I need two arguments, got %d\n", argc - 1);
    result = -1;
    goto end; // the one and only case where a goto is accepted!
  }
  // convert args from strings to numbers
  for (i = 0; i < 2; i++) {
    factor[i] = atof(argv[i + 1]);
  }
  // multiply the two arguments
  result = factor[0] * factor[1];
  // print the result
  printf("%f * %f = %f\n", factor[0], factor[1], result);

end:
  // a single exit point
  return result;
}