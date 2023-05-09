#include "../defines.h"
#include "../fsm.h"
#include <stdint.h>
#include <unistd.h>

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
    // TODO: implement a timing strategy
    usleep(5000);
  } while (cur_state != CCNC_STATE_STOP);
  ccnc_run_state(cur_state, &state_data);
  return 0;
}
