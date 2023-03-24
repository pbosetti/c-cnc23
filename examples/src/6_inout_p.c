#include "inout.h"

int main() {
  int n = 10;

  // First example
  float *a;
  printf("A address is: %p\n\n", a);

  a = f1(n);
  array_print(a, n);
  printf("A address is: %p\n\n", a);

  // Second example
  float *b = NULL;
  f2(b, n);
  printf("B address is: %p\n\n", b);
  // array_print(b, n); // this would segfault! (why?)

  // Third example 
  float *c = NULL;
  f3(&c, n);
  array_print(c, n);
  printf("C address is: %p\n\n", c);

  free(a);
  free(b);
  free(c);

  return 0;

} // end of main
