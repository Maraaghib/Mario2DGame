#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "timer.h"

// Return number of elapsed µsec since... a long time ago
static unsigned long get_time (void)
{
  struct timeval tv;

  gettimeofday (&tv ,NULL);

  // Only count seconds since beginning of 2016 (not jan 1st, 1970)
  tv.tv_sec -= 3600UL * 24 * 365 * 46;
  
  return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN

void foe (int param) { // TODO : Complete and rename it later
  printf("sdl_push_event(%p) appelee au temps %ld\n", param, get_time()); // NOT READY
  // TODO
}

void *handler (*void arg) {
  struct sigaction sa;
  sa.sa_handler = foe;// TODO : Rename it later
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigset_t mask;
  sigset_t empty_mask;

  sigemptyset(&mask);
  sigemptyset(&empty_mask);
  sigaction(SIGALARM, &sa, NULL);

  while(1) {
    sigsuspend(&empty_mask);
  }
}  
  
// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGALARM);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  pthread_t pid = (pthread_t)NULL; 
  if (pthread_create(&pid, NULL, handler, (void *)pid) != 0) {
    perror("The thread creation failed !\n");
    exit(EXIT_FAILURE);
  }
  
  return 0; // Implementation not ready
}

void timer_set (Uint32 delay, void *param)
{
  // TODO
}

#endif
