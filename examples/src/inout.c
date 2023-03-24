#include "inout.h"

void array_print(float *a, int n){
  printf("[");
  int i;
  for (i=0; i<n; i++){
    printf(" %f ", a[i]);
  }
  printf("]\n");
}

float *f1(int n) {
  float *a = malloc(n*sizeof(float));
  printf("f1(%i) - a address is: %p\n", n, a);
  int i;
  for(i=0; i<n; i++){
    a[i] = i;
  }
  return a;
}

void f2(float *a, int n) {
  a = malloc(n*sizeof(float));
  printf("f2(%i) - a address is: %p\n", n, a);
  for(int i=0; i<n; i++) {
    a[i] = i;
  }
  array_print(a, n);
}

void f3(float **a, int n) {
  *a = malloc(n*sizeof(float));
  printf("f3(%i) - a address is: %p\n", n, *a);
  int i;
  for(i=0; i<n; i++) {
    (*a)[i] = i;
  }
}