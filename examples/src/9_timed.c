//   _____ _                    _   _                     ____  
//  |_   _(_)_ __ ___   ___  __| | | | ___   ___  _ __   |___ \
//    | | | | '_ ` _ \ / _ \/ _` | | |/ _ \ / _ \| '_ \    __) |
//    | | | | | | | | |  __/ (_| | | | (_) | (_) | |_) |  / __/ 
//    |_| |_|_| |_| |_|\___|\__,_| |_|\___/ \___/| .__/  |_____|
//                                               |_|            
// This version uses a SIGALRM signal, raised by the kernel at regular 
// intervals. A call to sleep() returns after the given time has elapsed
// or after receiving an ALRM signal, whichever first.
// This version does not need any tuning of the step time and it is typically
// the best way to accomplish the task. On Realtime kernels the time jitter 
// can be about +-1 us, on normal kernels about +-1 ms
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// an empty signal handler
void handler(int signal) {}

int main(int argc, char const **argv) {
  int i = 0;
  double t0 = 0, t = 0, dt = 0;
  struct itimerval itv;
  struct timespec ts;

  // the itimer interval needs two time periods: it_value is the time before
  // the first occurrence, it_interval is the interval between further
  // occurences
  itv.it_interval.tv_sec = 0;
  itv.it_interval.tv_usec = 5000;
  itv.it_value.tv_sec = 0;
  itv.it_value.tv_usec = 5000;

  // install the signal handler for SIGALRM
  signal(SIGALRM, handler);
  // set the timer
  if (setitimer(ITIMER_REAL, &itv, NULL)) {
    perror("Error setting timer");
  }
  
  // initialize the clock
  clock_gettime(CLOCK_MONOTONIC, &ts);
  t0 = ts.tv_sec + ts.tv_nsec / 1.0E9;

  // header
  printf("n, dt\n");
  
  // main loop
  for (i = 0; i < 1000; i++) {
    // the sleep waits for the next SIGALRM, but no more than 1 s
    sleep(1);
    // calculate and print the delta time
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = ts.tv_sec + ts.tv_nsec / 1.0E9;
    dt = t - t0;
    t0 = t;
    printf("%03d, %f\n", i, dt);
    // here we do our business, which is supposed to take less than 5000 us
  }

  return 0;
}