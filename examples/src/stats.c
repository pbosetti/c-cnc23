#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int runstats(double x, size_t n, double *mean, double *sd) {
  if (n <= 1) { // recursion formula: first element (base-1)
    *mean = x;
    *sd = 0;
  } else {
    const double n1 = n - 1;
    const double n2 = n - 2;
    const double nr = 1.0 / n;
    const double n1r = 1.0 / n1;
    const double nn1 = n / n1;
    *mean = nr * (n1 * *mean + x);
    *sd = sqrt(n1r * (n2 * pow(*sd, 2) + nn1 * pow(*mean - x, 2)));
  }
  return n;
}


#ifdef STATS_MAIN

int main(int argc, const char **argv) {
  int i;
  double mean = 0, sd = 0;
  for (i = 1; i < argc; i++) {
    runstats(atof(argv[i]), i, &mean, &sd);
  }
  printf("%d values: mean %.9f, sd %.9f\n", i-1, mean, sd);
}

#endif