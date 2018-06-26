
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define INTERVAL 100 /* microseconds */
#define DURATION 10   /* seconds */
 
void signalhandler(int signo)
{
  /* Do something every INTERVAL microseconds */
  /* For example, write a capital A to stdout */
  char c = 'A';
  write (1, &c, 1);
}





int main(int argc, char *argv[])
{
    struct sigaction action;
    struct itimerval timer;
    struct timespec ts, rem;
    struct timeval end, now;
    struct sched_param schp;

//  set priority
    memset(&schp,0,sizeof(schp)); 
    schp.sched_priority = sched_get_priority_max(SCHED_FIFO); 
    if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0) {
      printf("cyclic reports: error sched_setscheduler");
      usleep(2000000);
      return -1;
    }


  /* Catch SIGALRM */
  action.sa_handler = signalhandler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  sigaction (SIGALRM, &action, NULL);
 
  /* Send SIGALRM every INTERVAL microseconds */
  timer.it_value.tv_sec = timer.it_interval.tv_sec = 0;
  timer.it_value.tv_usec = timer.it_interval.tv_usec = INTERVAL;
  setitimer(ITIMER_REAL, &timer, NULL);

  /* Define sleep parameters */ 
  ts.tv_sec = DURATION;
  ts.tv_nsec = 0;
  rem = ts;
 
  /* Calculate end time of program run */
  gettimeofday(&end, NULL);
  end.tv_sec += DURATION;
 
  while (rem.tv_sec || rem.tv_nsec) {
    /* Continue sleep, if woken up by our own signal */
    if (nanosleep(&ts, &rem) == -1 && errno == EINTR)
      ts = rem;
    else
      break;
    /* DURATION elapsed? */
    gettimeofday(&now, NULL);
    if (now.tv_sec >= end.tv_sec && now.tv_usec >= end.tv_usec)
      break;
  }
 
  return(0);
}
