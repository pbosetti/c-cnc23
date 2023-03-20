//   _____ _                    _   _                        _ 
//  |_   _(_)_ __ ___   ___  __| | | |    ___   ___  _ __   / |
//    | | | | '_ ` _ \ / _ \/ _` | | |   / _ \ / _ \| '_ \  | |
//    | | | | | | | | |  __/ (_| | | |__| (_) | (_) | |_) | | |
//    |_| |_|_| |_| |_|\___|\__,_| |_____\___/ \___/| .__/  |_|
//                                                  |_|        
// This version uses a timing thread, which is started at the beginning of 
// each loop and joined at the end.
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // usleep
#include <time.h>

// Timing thread task
// It simply sleeps for the required time. Given that creating a thread is
// relatively expensive, the nominal time has to be compensated for the average
// time for creating a thread, which must be determined exprimentally
// and is system-dependent
void *wait_thread(void *arg) {
  usleep(5000); // us seconds sleep time
  // usleep(5000 * 0.95); // us seconds sleep time
  return NULL;
}

int main(int argc, char const **argv) {
  pthread_t pt1;
  double t0=0, t, dt;
  struct timespec ts;
  int rc, i;

  // Get current time
  clock_gettime(CLOCK_MONOTONIC, &ts);
  t0 = ts.tv_sec + ts.tv_nsec / 1.0E9;

  fprintf(stdout, "n, dt\n");

  for(i=0; i<1000; i++) {
    // Lets create the thread
    rc = pthread_create(&pt1, NULL, wait_thread, NULL);
    // Meanwhile, we're doing our stuff that is supposed to take less than
    // the thread net time.
    // Calculate and print the delta time
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = ts.tv_sec + ts.tv_nsec / 1.0E9;
    dt = t - t0;
    t0 = t;

    fprintf(stdout, "%d, %f\n", i, dt);
    // Lets do some work that takes 100 us
    usleep(100);

    // Finally, wait for the timing thread to elapse and return
    pthread_join(pt1, NULL);
  }

  return 0;

}