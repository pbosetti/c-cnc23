#include "../defines.h"
#include "../fsm.h"
#include <unistd.h>
#include <sys/time.h>
#include <sched.h>
#include <signal.h>

#define INI_FILE "machine.ini"

// empty signal handler
void handler(int signal) {}


int main(int argc, char const *argv[]) {
  struct itimerval itv;
  ccnc_state_data_t state_data = {
    .ini_file = INI_FILE,
    .prog_file = (char *)argv[1],
    .machine = machine_new(INI_FILE),
    .program = NULL
  };
  ccnc_state_t cur_state = CCNC_STATE_INIT;
  data_t rt_pacing = machine_rt_pacing(state_data.machine);
  useconds_t dt = machine_tq(state_data.machine) * 1E6 / rt_pacing;
  useconds_t dt_max = dt * 10;

  if (!state_data.machine) {
    eprintf("Error initializing the machine\n");
    exit(EXIT_FAILURE);
  }

  // On Linux, set the scheduler and the priority
  #ifdef __linux__
  struct sched_param param = {.sched_priority = 80};
  if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) {
    perror("Error (need sudo?): ");
    exit(EXIT_FAILURE);
  }
  #endif

  // set the timer intervals
  itv.it_interval.tv_sec = 0;
  itv.it_interval.tv_usec = dt;
  itv.it_value.tv_sec = 0;
  itv.it_value.tv_usec = dt;

  // install signal handler
  signal(SIGALRM, handler);

  // load the timer mechanism
  if (setitimer(ITIMER_REAL, &itv, NULL)) {
    eprintf("Could not set timer\n");
    exit(EXIT_FAILURE);
  }

  // Setup system logging
  openlog("SM", LOG_PID | LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "Starting SM");

  // MAIN LOOP
  do {
    cur_state = ccnc_run_state(cur_state, &state_data);
    // sleep for AT MOST dt_max
    if (usleep(dt_max) == 0) {
      wprintf("Did not complete the loop in less than %d\n", dt_max);
    }
  } while (cur_state != CCNC_STATE_STOP);
  ccnc_run_state(cur_state, &state_data);
  return 0;
}