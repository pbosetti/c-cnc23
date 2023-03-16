#include <stdio.h>
#include <stdlib.h>

#include "struct_union.h"

int main() {
  // declaration and definition of a structure instance:
  point_t pt = {.x = 0, .y = 0, .z = 0};
  // equivalent to:
  // point_t pt;
  // pt.x = 0;
  // pt.y = 0;
  // pt.z = 0;
  my_union another_pt;

  // pointer to pt:
  point_t *pt_ptr = &pt;
  pt.y = 20;
  pt_ptr->x = 10;

  // this function is declared in the header struct_union.h and defined after
  // this main
  printf("pt_ptr: ");
  point_print(pt_ptr);
  printf("pt:     ");
  point_print(&pt);

  another_pt.x = 10;
  another_pt.y = 20;

  printf("Size of union: %lu bytes\n", sizeof(another_pt));
  printf("Size of pt:    %lu bytes\n", sizeof(pt));

  printf("another_pt.x:  %.3f\n", another_pt.x);
  printf("another_pt.y:  %.3f\n", another_pt.y); // actually the same value!
  printf("union bytes: ");
  print_bytes(another_pt.str, sizeof(another_pt));

  {
    data_t ary[ARY_LEN] = {10, 2}; // remaining 4 values are not initialized!
    print_array(ary, ARY_LEN);
  }

  return 0;
}

void point_print(point_t *a) { 
  printf("(%.3f %.3f %.3f)\n", a->x, a->y, a->z);
}

void print_array(data_t a[], int n) {
  printf("[");
  for (int i = 0; i < n; i++) {
    printf("%.3f ", a[i]);
  }
  printf("\b]\n"); // \b is backspace: deletes last space in excess
}

void print_bytes(unsigned char const *str, int len) {
  int i;
  for (i = 0; i < len; i++) {
    printf("%02x:", str[i]); // %02x means hexadecimal, 2 digits, 0-padded left
  }
  printf("\b (%d bytes)\n", len);
}