#include "../defines.h"
#include "../fsm.h"
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

#include <unistd.h> // Sleep
static uint64_t now_ns() {
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
  ccnc_state_data_t state_data = {
    .ini_file = "machine.ini",
    .prog_file = (char *)argv[1],
    .machine = NULL,
    .prog = NULL
  };
  ccnc_state_t cur_state = CCNC_STATE_INIT;
  openlog("SM", LOG_PID | LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "Starting SM");
  do {
    cur_state = ccnc_run_state(cur_state, &state_data);
    // wait_next(machine_tq(state_data.machine) * 1E9 / machine_rt_pacing(state_data.machine));
    usleep(5000);
  } while (cur_state != CCNC_STATE_STOP);
  ccnc_run_state(cur_state, &state_data);
  return 0;
}
