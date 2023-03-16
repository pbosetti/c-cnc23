#include <stdint.h>
#if defined(__linux)
#define HAVE_POSIX_TIMER
#include <time.h>
#ifdef CLOCK_MONOTONIC
#define CLOCKID CLOCK_MONOTONIC
#else
#define CLOCKID CLOCK_REALTIME
#endif
#elif defined(__APPLE__)
#include <time.h>
#define HAVE_MACH_TIMER
#include <mach/mach_time.h>
#endif
#include <stdio.h>
#include <unistd.h>

uint64_t now_ns() {
  static uint64_t is_init = 0;
#if defined(__APPLE__)
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
#elif defined(__linux)
  static struct timespec linux_rate;
  if (0 == is_init) {
    clock_getres(CLOCKID, &linux_rate);
    is_init = 1;
  }
  uint64_t now;
  struct timespec spec;
  clock_gettime(CLOCKID, &spec);
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


int main(int argc, char const *argv[]) {
  double t0 = 0, t = 0, dt = 0;
  struct timespec ts;
  int rc, i = 0;

  // initialize the clock
  clock_gettime(CLOCK_MONOTONIC, &ts);
  t0 = ts.tv_sec + ts.tv_nsec / 1.0E9;

  // header
  printf("n, dt\n");
  
  // main loop
  for (i = 0; i < 1000; i++) {
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = ts.tv_sec + ts.tv_nsec / 1.0E9;
    dt = t - t0;
    t0 = t;
    printf("%03d, %f\n", i, dt);
    // let's pretend to do domething that takes 100 us
    usleep(100);
    wait_next(5e6);
  }

  return 0;
}