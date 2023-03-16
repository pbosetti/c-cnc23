//   _____                     __                      
//  |_   _| __ __ _ _ __  ___ / _| ___  _ __ _ __ ___  
//    | || '__/ _` | '_ \/ __| |_ / _ \| '__| '_ ` _ \
//    | || | | (_| | | | \__ \  _| (_) | |  | | | | | |
//    |_||_|  \__,_|_| |_|___/_|  \___/|_|  |_| |_| |_|
// 
// Roto-translation of a point in 3D
// Rotation is about Z axis on XY plane
// Build with:
//    clang -lgsl -lm -o transform src/transform.c
// If you want to use gcc, the order MATTERS:
//    gcc src/transform.c -lgsl -lm -o transform
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// point object that we are going to use in C-CNC
typedef struct {
  double x, y, z;
} point_t;

// transform point with angle rotation followed by delta translation
// the transformed coordinate is in point
int transform(point_t *point, double angle, point_t *delta) {
  gsl_matrix *T = gsl_matrix_alloc(4, 4);
  gsl_vector *p = gsl_vector_alloc(4);
  gsl_vector *p1 = gsl_vector_alloc(4);
  int rv;

  double th = angle / 180.0 * M_PI;
  double sa = sin(th);
  double ca = cos(th);

  gsl_vector_set_all(p, 1);
  gsl_vector_set(p, 0, point->x);
  gsl_vector_set(p, 1, point->y);
  gsl_vector_set(p, 2, point->z);

  gsl_matrix_set_identity(T);
  // first row
  gsl_matrix_set(T, 0, 0, ca);
  gsl_matrix_set(T, 0, 1, -sa);
  gsl_matrix_set(T, 0, 3, delta->x);
  // second row
  gsl_matrix_set(T, 1, 0, sa);
  gsl_matrix_set(T, 1, 1, ca);
  gsl_matrix_set(T, 1, 3, delta->y);
  // third row
  gsl_matrix_set(T, 2, 3, delta->z);

  // BLAS level 2: GEneral Matrix-Vector product (Double)
  rv = gsl_blas_dgemv(CblasNoTrans, 1, T, p, 0, p1);

  point->x = gsl_vector_get(p1, 0);
  point->y = gsl_vector_get(p1, 1);
  point->z = gsl_vector_get(p1, 2);

  gsl_vector_free(p);
  gsl_vector_free(p1);
  gsl_matrix_free(T);
  return rv;
}


int main(int argc, char const *argv[]) {
  point_t pt = {.x=10, .y=0, .z=0};
  point_t d = {.x=10, .y=10, .z=5};
  double alpha = 45.0;

  printf("Roto-translation transform in XY plane\n");
  printf("Before:      (%8.4f, %8.4f, %8.4f)\n", pt.x, pt.y, pt.z);
  printf("Translation: (%8.4f, %8.4f, %8.4f)\n", d.x, d.y, d.z);
  printf("Rotation:    %+8.3f°\n", alpha);
  transform(&pt, alpha, &d);
  printf("After:       (%8.4f, %8.4f, %8.4f)\n", pt.x, pt.y, pt.z);

  return 0;
}
