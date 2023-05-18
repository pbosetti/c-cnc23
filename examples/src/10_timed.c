//   _____ _                    _   _                     _____ 
//  |_   _(_)_ __ ___   ___  __| | | | ___   ___  _ __   |___ / 
//    | | | | '_ ` _ \ / _ \/ _` | | |/ _ \ / _ \| '_ \    |_ \
//    | | | | | | | | |  __/ (_| | | | (_) | (_) | |_) |  ___) |
//    |_| |_|_| |_| |_|\___|\__,_| |_|\___/ \___/| .__/  |____/ 
//                                               |_|            
// Explicit wait loop: more precise, but higher CPU load
#include <stdint.h>
#include <time.h>
#ifdef __APPLE__
#include <mach/mach_time.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>


uint64_t now_ns() {
  static uint64_t is_init = 0;
#ifdef __APPLE__
  static mach_timebase_info_data_t info;
  if (0 == is_init) {
    mach_timebase_info(&info);
    is_init = 1;
  }
  uint64_t now;
  now = mach_absolute_time();
  now *= info.numer;
  now /= info.denom;
  return now;
#elif defined __linux__
  static struct timespec linux_rate;
  if (0 == is_init) {
    clock_getres(CLOCK_MONOTONIC, &linux_rate);
    is_init = 1;
  }
  uint64_t now;
  struct timespec spec;
  clock_gettime(CLOCK_MONOTONIC, &spec);
  now = spec.tv_sec * 1.0e9 + spec.tv_nsec;
  return now;
#endif
}


uint64_t wait_next(uint64_t interval) {
  static uint64_t last_call = 0;
  uint64_t delta, delay;
  if (last_call == 0 || interval == 0) last_call = now_ns();
  while ((delta = now_ns() - last_call)) {
    if (delta >= interval) {
      delay = delta - interval;
      last_call = now_ns() - delay;
      return delay;
    }
  }
  return 0;
}

extern int runstats(double x, size_t n, double *mean, double *sd);


int main(int argc, char const **argv) {
  double t0 = 0, t = 0, dt = 0;
  struct timespec ts;
  int rc, i = 0;
  double mean = 0, sd = 0;
  #ifdef __linux__
  struct sched_param param = {.sched_priority = 80};
  rc = sched_setscheduler(getpid(), SCHED_FIFO, &param);
  if (rc) {
    perror("Error (need sudo?): ");
    exit(EXIT_FAILURE);
  }
  #endif

  // initialize the clock
  clock_gettime(CLOCK_MONOTONIC, &ts);
  t0 = ts.tv_sec + ts.tv_nsec / 1.0E9;

  // header
  fprintf(stderr, "n, dt\n");
  
  // main loop
  for (i = 0; i < 1000; i++) {
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = ts.tv_sec + ts.tv_nsec / 1.0E9;
    dt = t - t0;
    t0 = t;
    runstats(dt, i + 1, &mean, &sd);
    fprintf(stderr, "%03d, %.9f\n", i, dt);
    // let's pretend to do domething that takes 100 us
    usleep(100);
    wait_next(5e6);
  }
  printf("Timestep: mean %.9f, sd %.9f\n", mean, sd);

  return 0;
}