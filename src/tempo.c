#define _XOPEN_SOURCE 
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

struct event {
  void* parameter;
  unsigned long timer;
  struct itimerval delay; 
};

struct event tab[100];
int iterator = 0;

void foe (int param) { 
  sdl_push_event(tab[0].parameter);
  for (int i = 0 ; i < iterator + 1; i++) {
    tab[i].timer = tab[i+1].timer;
    tab[i].parameter = tab[i+1].parameter;
	tab[i].delay.it_value.tv_sec = tab[i+1].delay.it_value.tv_sec; 
	tab[i].delay.it_value.tv_usec = tab[i+1].delay.it_value.tv_usec; 
  }
  iterator-=1;
}

void *handler (void *arg) {
  struct sigaction sa;
  sa.sa_handler = foe;// TODO : Rename it later
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigset_t mask;
  sigset_t empty_mask;

  sigemptyset(&mask);
  sigemptyset(&empty_mask);
  sigaction(SIGALRM, &sa, NULL);

  while(1) {
    sigsuspend(&empty_mask);
  }
}  

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGALRM);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  pthread_t pid = (pthread_t)NULL; 
  if (pthread_create(&pid, NULL, handler, (void *)pid) != 0) {
    perror("The thread creation failed !\n");
    exit(EXIT_FAILURE);
  }
  
  return 1; // Implementation ready
}

void timer_set (Uint32 delay, void *param) {   
  unsigned long int delay_sec = delay / 1000;
  unsigned long int delay_usec = (delay % 1000) * 1000;

  struct event* e = malloc(sizeof(struct event));
  e->delay.it_value.tv_sec = delay_sec;
  e->delay.it_value.tv_usec = delay_usec;
  e->delay.it_interval.tv_sec = 0;
  e->delay.it_interval.tv_usec = 0;
  e->timer = get_time();
  e->parameter = param;

  for (int i = 0; i < 100; i++) { 
	  if (tab[i].parameter == NULL) {
			tab[i] = *e;
	  }
  }
 	
  if (tab[0].timer == e->timer && tab[0].parameter == e->parameter ) {
      setitimer(ITIMER_REAL, &e->delay, NULL);
  }  
}

#endif
