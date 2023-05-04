/******************************************************************************
Finite State Machine
Project: C-CNC
Description: Finite state machine for C-CNC

Generated by gv_fsm ruby gem, see https://rubygems.org/gems/gv_fsm
gv_fsm version 0.3.4
Generation date: 2023-05-04 13:18:54 +0200
Generated from: src/fsm.dot
The finite state machine has:
  7 states
  4 transition functions
Functions and types have been generated with prefix "ccnc_"
******************************************************************************/

#include <syslog.h>
#include "fsm.h"

// Install signal handler: 
// SIGINT requests a transition to state stop
#include <signal.h>
static int _exit_request = 0;
static void signal_handler(int signal) {
  if (signal == SIGINT) {
    _exit_request = 1;
    syslog(LOG_WARNING, "[FSM] SIGINT transition to stop");
  }
}

// SEARCH FOR Your Code Here FOR CODE INSERTION POINTS!

// GLOBALS
// State human-readable names
const char *ccnc_state_names[] = {"init", "idle", "stop", "load_block", "no_motion", "rapid_motion", "interp_motion"};

// List of state functions
state_func_t *const ccnc_state_table[CCNC_NUM_STATES] = {
  ccnc_do_init,          // in state init
  ccnc_do_idle,          // in state idle
  ccnc_do_stop,          // in state stop
  ccnc_do_load_block,    // in state load_block
  ccnc_do_no_motion,     // in state no_motion
  ccnc_do_rapid_motion,  // in state rapid_motion
  ccnc_do_interp_motion, // in state interp_motion
};

// Table of transition functions
transition_func_t *const ccnc_transition_table[CCNC_NUM_STATES][CCNC_NUM_STATES] = {
  /* states:           init             , idle             , stop             , load_block       , no_motion        , rapid_motion     , interp_motion     */
  /* init          */ {NULL             , NULL             , NULL             , NULL             , NULL             , NULL             , NULL             }, 
  /* idle          */ {NULL             , NULL             , NULL             , ccnc_reset       , NULL             , NULL             , NULL             }, 
  /* stop          */ {NULL             , NULL             , NULL             , NULL             , NULL             , NULL             , NULL             }, 
  /* load_block    */ {NULL             , NULL             , NULL             , NULL             , NULL             , ccnc_begin_rapid , ccnc_begin_interp}, 
  /* no_motion     */ {NULL             , NULL             , NULL             , NULL             , NULL             , NULL             , NULL             }, 
  /* rapid_motion  */ {NULL             , NULL             , NULL             , ccnc_end_rapid   , NULL             , NULL             , NULL             }, 
  /* interp_motion */ {NULL             , NULL             , NULL             , NULL             , NULL             , NULL             , NULL             }, 
};

/*  ____  _        _       
 * / ___|| |_ __ _| |_ ___ 
 * \___ \| __/ _` | __/ _ \
 *  ___) | || (_| | ||  __/
 * |____/ \__\__,_|\__\___|
 *                         
 *   __                  _   _                 
 *  / _|_   _ _ __   ___| |_(_) ___  _ __  ___ 
 * | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 * |  _| |_| | | | | (__| |_| | (_) | | | \__ \
 * |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 */                                             

// Function to be executed in state init
// valid return states: CCNC_STATE_IDLE, CCNC_STATE_STOP
ccnc_state_t ccnc_do_init(ccnc_state_data_t *data) {
  ccnc_state_t next_state = CCNC_STATE_IDLE;
  signal(SIGINT, signal_handler); 
  
  syslog(LOG_INFO, "[FSM] In state init");
  /* Your Code Here */
  
  switch (next_state) {
    case CCNC_STATE_IDLE:
    case CCNC_STATE_STOP:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from init to %s, remaining in this state", ccnc_state_names[next_state]);
      next_state = CCNC_NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state idle
// valid return states: CCNC_NO_CHANGE, CCNC_STATE_IDLE, CCNC_STATE_LOAD_BLOCK, CCNC_STATE_STOP
// SIGINT triggers an emergency transition to stop
ccnc_state_t ccnc_do_idle(ccnc_state_data_t *data) {
  ccnc_state_t next_state = CCNC_NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state idle");
  /* Your Code Here */
  
  switch (next_state) {
    case CCNC_NO_CHANGE:
    case CCNC_STATE_IDLE:
    case CCNC_STATE_LOAD_BLOCK:
    case CCNC_STATE_STOP:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from idle to %s, remaining in this state", ccnc_state_names[next_state]);
      next_state = CCNC_NO_CHANGE;
  }
  
  // SIGINT transition override
  if (_exit_request) next_state = CCNC_STATE_STOP;
  
  return next_state;
}


// Function to be executed in state stop
// valid return states: CCNC_NO_CHANGE
ccnc_state_t ccnc_do_stop(ccnc_state_data_t *data) {
  ccnc_state_t next_state = CCNC_NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state stop");
  /* Your Code Here */
  
  switch (next_state) {
    case CCNC_NO_CHANGE:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from stop to %s, remaining in this state", ccnc_state_names[next_state]);
      next_state = CCNC_NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state load_block
// valid return states: CCNC_STATE_IDLE, CCNC_STATE_NO_MOTION, CCNC_STATE_RAPID_MOTION, CCNC_STATE_INTERP_MOTION
ccnc_state_t ccnc_do_load_block(ccnc_state_data_t *data) {
  ccnc_state_t next_state = CCNC_STATE_IDLE;
  
  syslog(LOG_INFO, "[FSM] In state load_block");
  /* Your Code Here */
  
  switch (next_state) {
    case CCNC_STATE_IDLE:
    case CCNC_STATE_NO_MOTION:
    case CCNC_STATE_RAPID_MOTION:
    case CCNC_STATE_INTERP_MOTION:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from load_block to %s, remaining in this state", ccnc_state_names[next_state]);
      next_state = CCNC_NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state no_motion
// valid return states: CCNC_STATE_LOAD_BLOCK
ccnc_state_t ccnc_do_no_motion(ccnc_state_data_t *data) {
  ccnc_state_t next_state = CCNC_STATE_LOAD_BLOCK;
  
  syslog(LOG_INFO, "[FSM] In state no_motion");
  /* Your Code Here */
  
  switch (next_state) {
    case CCNC_STATE_LOAD_BLOCK:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from no_motion to %s, remaining in this state", ccnc_state_names[next_state]);
      next_state = CCNC_NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state rapid_motion
// valid return states: CCNC_NO_CHANGE, CCNC_STATE_LOAD_BLOCK, CCNC_STATE_RAPID_MOTION
// SIGINT triggers an emergency transition to stop
ccnc_state_t ccnc_do_rapid_motion(ccnc_state_data_t *data) {
  ccnc_state_t next_state = CCNC_NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state rapid_motion");
  /* Your Code Here */
  
  switch (next_state) {
    case CCNC_NO_CHANGE:
    case CCNC_STATE_LOAD_BLOCK:
    case CCNC_STATE_RAPID_MOTION:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from rapid_motion to %s, remaining in this state", ccnc_state_names[next_state]);
      next_state = CCNC_NO_CHANGE;
  }
  
  // SIGINT transition override
  if (_exit_request) next_state = CCNC_STATE_STOP;
  
  return next_state;
}


// Function to be executed in state interp_motion
// valid return states: CCNC_NO_CHANGE, CCNC_STATE_LOAD_BLOCK, CCNC_STATE_INTERP_MOTION
// SIGINT triggers an emergency transition to stop
ccnc_state_t ccnc_do_interp_motion(ccnc_state_data_t *data) {
  ccnc_state_t next_state = CCNC_NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state interp_motion");
  /* Your Code Here */
  
  switch (next_state) {
    case CCNC_NO_CHANGE:
    case CCNC_STATE_LOAD_BLOCK:
    case CCNC_STATE_INTERP_MOTION:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from interp_motion to %s, remaining in this state", ccnc_state_names[next_state]);
      next_state = CCNC_NO_CHANGE;
  }
  
  // SIGINT transition override
  if (_exit_request) next_state = CCNC_STATE_STOP;
  
  return next_state;
}


/*  _____                    _ _   _              
 * |_   _| __ __ _ _ __  ___(_) |_(_) ___  _ __   
 *   | || '__/ _` | '_ \/ __| | __| |/ _ \| '_ \
 *   | || | | (_| | | | \__ \ | |_| | (_) | | | | 
 *   |_||_|  \__,_|_| |_|___/_|\__|_|\___/|_| |_| 
 *                                                
 *   __                  _   _                 
 *  / _|_   _ _ __   ___| |_(_) ___  _ __  ___ 
 * | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 * |  _| |_| | | | | (__| |_| | (_) | | | \__ \
 * |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 */    
                                         
// This function is called in 1 transition:
// 1. from idle to load_block
void ccnc_reset(ccnc_state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition ccnc_reset");
  /* Your Code Here */
}

// This function is called in 1 transition:
// 1. from load_block to rapid_motion
void ccnc_begin_rapid(ccnc_state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition ccnc_begin_rapid");
  /* Your Code Here */
}

// This function is called in 1 transition:
// 1. from load_block to interp_motion
void ccnc_begin_interp(ccnc_state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition ccnc_begin_interp");
  /* Your Code Here */
}

// This function is called in 1 transition:
// 1. from rapid_motion to load_block
void ccnc_end_rapid(ccnc_state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition ccnc_end_rapid");
  /* Your Code Here */
}


/*  ____  _        _        
 * / ___|| |_ __ _| |_ ___  
 * \___ \| __/ _` | __/ _ \
 *  ___) | || (_| | ||  __/ 
 * |____/ \__\__,_|\__\___| 
 *                          
 *                                              
 *  _ __ ___   __ _ _ __   __ _  __ _  ___ _ __ 
 * | '_ ` _ \ / _` | '_ \ / _` |/ _` |/ _ \ '__|
 * | | | | | | (_| | | | | (_| | (_| |  __/ |   
 * |_| |_| |_|\__,_|_| |_|\__,_|\__, |\___|_|   
 *                              |___/           
 */

ccnc_state_t ccnc_run_state(ccnc_state_t cur_state, ccnc_state_data_t *data) {
  ccnc_state_t new_state = ccnc_state_table[cur_state](data);
  if (new_state == CCNC_NO_CHANGE) new_state = cur_state;
  transition_func_t *transition = ccnc_transition_table[cur_state][new_state];
  if (transition)
    transition(data);
  return new_state;
};

#ifdef TEST_MAIN
#include <unistd.h>
int main() {
  ccnc_state_t cur_state = CCNC_STATE_INIT;
  openlog("SM", LOG_PID | LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "Starting SM");
  do {
    cur_state = ccnc_run_state(cur_state, NULL);
    sleep(1);
  } while (cur_state != CCNC_STATE_STOP);
  ccnc_run_state(cur_state, NULL);
  return 0;
}
#endif
