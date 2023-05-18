#include "../defines.h"
#include "../fsm.h"
#include "../machine.h"
#include <unistd.h>
#include <sys/time.h>

// an empty signal handler
void handler(int signal) {}

#define INI_FILE "machine.ini"

int main(int argc, char const *argv[]) {
  struct itimerval itv;
  ccnc_state_data_t state_data = {
    .ini_file = INI_FILE,
    .prog_file = (char *)argv[1],
    .machine = machine_new(INI_FILE),
    .program = NULL
  };
  ccnc_state_t cur_state = CCNC_STATE_INIT;
  suseconds_t dt = machine_tq(state_data.machine)*1E6;
  useconds_t dt_max = dt * 10;

  #ifdef __linux__
  struct sched_param param = {.sched_priority = 80};
  rc = sched_setscheduler(getpid(), SCHED_FIFO, &param);
  if (rc) {
    eprintf("Error (need sudo?): ");
    exit(EXIT_FAILURE);
  }
  #endif

  // the itimer interval needs two time periods: it_value is the time before
  // the first occurrence, it_interval is the interval between further
  // occurences
  itv.it_interval.tv_sec = 0;
  itv.it_interval.tv_usec = dt;
  itv.it_value.tv_sec = 0;
  itv.it_value.tv_usec = dt;

  // install the signal handler for SIGALRM
  signal(SIGALRM, handler);
  // set the timer
  if (setitimer(ITIMER_REAL, &itv, NULL)) {
    perror(BRED"Error setting timer"CRESET);
    exit(EXIT_FAILURE);
  }

  // setup system logging
  openlog("SM", LOG_PID | LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "Starting SM");

  // main loop
  do {
    cur_state = ccnc_run_state(cur_state, &state_data);
    // sleep for dt_max or until SIGALRM, whichever comes first
    if (usleep(dt_max) == 0) {
      wprintf("Did not get an alarm within %d us\n", dt_max);
    }
  } while (cur_state != CCNC_STATE_STOP);
  ccnc_run_state(cur_state, &state_data);
  return 0;
}
