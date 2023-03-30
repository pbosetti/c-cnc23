#include <stdio.h>
#include <stdlib.h>

#define ARY_LEN 6

// it's nuce to define aliases for common types, so that if later on we want to
// save memory bu using a float rather than a double, we can change a single
// line rather than all variable declarations
typedef double data_t;

typedef struct {
  data_t x, y, z;
} point_t;

typedef union {
  float x, y, z;
  unsigned char str[sizeof(float)];
} my_union;

void point_print(point_t *a);

void print_array(data_t a[], int n);

void print_bytes(unsigned char const *str, int len);