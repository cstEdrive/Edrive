
/*
  timer_posix.c

  Rene van de Molengraft, June, 2004
  revision history: June, 28th, 2004: start
                    October, 2nd, 2004: added rtc_ prefix to routine names
                    October, 23th, 2005: synchronized original timer to fusion-timer
                    October, 30th, 2005: added frequency check
                    October, 11th, 2007: changed to posix timer
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
//#include <sys/types.h>
//#include <sys/resource.h>
//#include <fcntl.h>
#include <sched.h>
#include <math.h>
#include "timer.h"





/* global variables */
static int imissed, icnt, istop, istart, igo;
static double avg_cpu_time, max_cpu_time, max_err, dt;
static double the_freq;
static double nominal_time;
static struct timeval tv0, tva;
static PTIMERCALLBACK the_user_apc;
static double tprev, dtmax=0.0, rem, dtnom, perc_missed;





int rtc_reset_time(void)
{
  gettimeofday(&tv0,NULL);

  return 0;
}





double rtc_get_time(void)
{
  double t,wrk;

  gettimeofday(&tva,NULL);

  t=(double) (tva.tv_sec-tv0.tv_sec)+((double) (tva.tv_usec-tv0.tv_usec))/1000000.0;

  return t;
}





double rtc_get_nominal_time(void)
{
  double t;

  t=nominal_time;

  return t;
}





int rtc_get_timer_stat(ptimer_stat pstat)
{
    pstat->icnt=(icnt-istart-1);
    pstat->imissed=imissed;
    pstat->avg_cpu_time=avg_cpu_time;
    pstat->max_cpu_time=max_cpu_time;
    pstat->max_err=max_err;
    pstat->dt=dt;

    return 0;
}





static double rtc_get_raw_time(void)
{
  double t;

  gettimeofday(&tva, NULL);

  t=(double) tva.tv_sec +((double) tva.tv_usec)/1000000.0;

  return t;
}





void signalhandler(int signo)
{
    double t, cpu_time, thit;

    //printf("Yep!\n");

    if (istop) {return;}
  
/*  reset time at first interrupt */
    if (icnt==istart) {rtc_reset_time();igo=1;nominal_time=-dtnom;}

      if (igo) {
/*      nominal time */
        nominal_time=nominal_time+dtnom;

/*      get actual time */
        t=rtc_get_time();

/*      call user apc */
        istop=the_user_apc(icnt-istart,imissed);

/*      check for overrun */
        if ( (icnt-istart)>1 ) {
          dt=tprev/(icnt-istart-1);
        }
        thit=(icnt-istart)*dt;

        imissed=imissed+(int) ((t-thit)/dt);

/*      save current time */
        tprev=t;

/*      compute statistics */
        cpu_time=rtc_get_time()-thit;
        avg_cpu_time=avg_cpu_time+cpu_time;
        if (cpu_time>max_cpu_time) {max_cpu_time=cpu_time;}
    }

/*  increase interrupt count */
    icnt++;


    return;
}





int rtc_sleep(int secs)
{
    rtc_usleep(1000000*secs);

    return 0;
}





#if 1
int rtc_usleep(int usecs)
{
    unsigned int dusecs;
    double t, te;
    
    t=rtc_get_raw_time();
    
    te=t+((double) usecs)*1.e-6;
    
    while (t<te) {
	dusecs=(int) ((te-t)*1.e6);
	usleep(dusecs);
	t=rtc_get_raw_time();
    }

    return 0;
}
#endif





#if 0
/*
there seems to be a problem with nanosleep and small sleep values, at least in the routine below
*/
int rtc_usleep(int usecs)
{
      struct timespec ts, rem;

      ts.tv_sec=(int) (usecs/1000000);
      ts.tv_nsec=(usecs-ts.tv_sec*1000000)*1000;
      //printf("secs = %d, nsecs = %d\n", ts.tv_sec, ts.tv_nsec);
      rem=ts;

      //printf("Sleeping...\n");
      while ( ts.tv_sec>0 || ts.tv_nsec>1000000 ) {
          /* Continue sleep, if woken up by our own signal */
          if (nanosleep(&ts, &rem) == -1 && errno == EINTR) {
              //printf("rem:  secs = %d, nsecs = %d\n", rem.tv_sec, rem.tv_nsec);
              memcpy(&ts, &rem, sizeof(struct timespec));
              //printf("ts:   secs = %d, nsecs = %d\n", ts.tv_sec, ts.tv_nsec);
	      rem.tv_sec=0;
	      rem.tv_nsec=0;
	      //printf("going to sleep again.\n");
          }
      }

      return 0;
}
#endif





int rtc_timer(double freq, PTIMERCALLBACK user_apc)
{
  double t;
  struct sched_param schp;

  struct sigaction action;
  struct itimerval timer;
  
  double us, secs;

/* check input timer frequency */
  if (freq<=0.0) {
    printf("rtc_timer (posix) reports: invalid timer frequency.\n");
    return -1;
  }

  the_freq=freq;
  the_user_apc=user_apc;

/* set priority */
  memset(&schp, 0, sizeof(schp)); 
  schp.sched_priority=sched_get_priority_max(SCHED_FIFO); 
  if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0) { 
    printf("rtc_timer (posix) reports: error sched_setscheduler");
    sleep(2);
    return -1;
  }

  printf("rtc_timer (posix) running at %f Hz.\n", the_freq);

/* FIX : 27-5-2010 Arthur Ketels */
/* 1/32th second is for timer to get stable */
  istart=((int) the_freq / 32) +1;
/* istart=((int) the_freq) +1; */

  tprev=0.0;
  istop=0;
  icnt=1;
  imissed=0;
  avg_cpu_time=0.0;
  max_cpu_time=0.0;
  max_err=0.0;
  dt=1.0/the_freq;
  dtnom=dt;
  igo=0;

/* start timer */
  /* Catch SIGALRM */
  action.sa_handler=signalhandler;
  sigemptyset(&action.sa_mask);
  action.sa_flags=0;
  sigaction(SIGALRM, &action, NULL);

  /* Send SIGALRM every INTERVAL microseconds */
  us=dt*1000000.0;
  secs=(double) ((int) (us/1000000.0));
  us=us-secs*1000000.0;
  //printf("s = %f, us = %f, dt = %f\n", secs, us, dt);
  
  timer.it_value.tv_sec=timer.it_interval.tv_sec=secs;
  timer.it_value.tv_usec=timer.it_interval.tv_usec=us;
  setitimer(ITIMER_REAL, &timer, NULL);

  while (!istop) {
    rtc_usleep(1000000);
  }

/* compute average cpu time */
  avg_cpu_time=avg_cpu_time/(icnt-istart-1);
  printf("rtc_timer (posix) reports:\n");
  perc_missed=100.0*((double) imissed)/((double) (icnt-istart-1));
  printf("missed interrupts: %6.3f (%d out of %d)\n",perc_missed,imissed,(icnt-istart-1));
  printf("average cpu-time used      : %f us\n",avg_cpu_time*1000000.00);
  printf("average cpu-time used      : %6.1f \n",100.0*(avg_cpu_time/dt));
  printf("maximum cpu-time used      : %6.1f \n",100.0*(max_cpu_time/dt));

  sleep(1);

  printf("rtc_timer (posix) has been stopped.\n");

  return 0;
}
