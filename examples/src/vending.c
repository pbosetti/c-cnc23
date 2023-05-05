// Example Finite state machine
// compile with:
// clang vending.c fsm.c -o vending -g
#include <stdlib.h> 
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include "fsm.h"

int main() {
  vm_state_data_t *data = malloc(sizeof(vm_state_data_t));
  memset(data, 0, sizeof(vm_state_data_t));
  vm_state_t cur_state = VM_STATE_INIT;
  openlog("SM", LOG_PID | LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "Starting SM");
  do {
    cur_state = vm_run_state(cur_state, data);
    usleep(200000);
  } while (cur_state != VM_STATE_STOP);
  vm_run_state(cur_state, data);
  return 0;
}