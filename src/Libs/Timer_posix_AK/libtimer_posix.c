
/*
  timer_posix.c

  Rene van de Molengraft, June, 2004
  revision history: June, 28th, 2004: start
                    October, 2nd, 2004: added rtc_ prefix to routine names
                    October, 23th, 2005: synchronized original timer to fusion-timer
                    October, 30th, 2005: added frequency check
                    October, 11th, 2007: changed to posix timer
                    June, 5th, 2009: Changed to posix timed wait by Arthur Ketels			
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
#include <sys/mman.h>
#include <sched.h>
#include <math.h>
#include "timer.h"

#define NSEC_PER_SEC 1000000000L
#define USEC_PER_SEC 1000000L
#define USEC_PER_SEC_F 1000000.0
#define NSEC_PER_SEC_F 1000000000.0

#define DEBUG

/* global variables */
static int imissed, icnt, istop, istart, igo;
static double avg_cpu_time, max_cpu_time, max_err, dt;
static double the_freq;
static double nominal_time;
static struct timespec tv0, tva;
static PTIMERCALLBACK the_user_apc;
static double tprev, dtnom, perc_missed; //dtmax=0.0, rem,
pthread_t thread1;
pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;




int rtc_reset_time(void)
{
  clock_gettime(CLOCK_REALTIME,&tv0);

  return 0;
}





double rtc_get_time(void)
{
  double t;

  clock_gettime(CLOCK_REALTIME, &tva);

  t=(double) (tva.tv_sec-tv0.tv_sec)+((double) (tva.tv_nsec-tv0.tv_nsec))/NSEC_PER_SEC_F;

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

  clock_gettime(CLOCK_REALTIME, &tva);

  t=(double) tva.tv_sec +((double) tva.tv_nsec)/NSEC_PER_SEC_F;

  return t;
}


void rtc_signalhandler(int signo)
{
    double t, cpu_time, thit;

    if (istop) {return;}
  
/*  reset time at first interrupt */
    if (icnt==istart) {rtc_reset_time();igo=1;nominal_time=-dtnom;}

      if (igo) {
/*      nominal time */
        nominal_time=nominal_time+dtnom;

#ifdef DEBUG
/*      get actual time */
        t=rtc_get_time();
#endif

/*      call user apc */
        istop=the_user_apc(icnt-istart,imissed);

#ifdef DEBUG
/*      check for overrun */
        if ( (icnt-istart)>1 ) {
          dt=tprev/(icnt-istart-1);
        }
        thit=(icnt-istart)*dt;

        imissed=imissed+(int) ((t-thit)/dt);

/*      save current time */
        tprev=t;

/*      compute statistics */
        cpu_time=rtc_get_time()-t;
        avg_cpu_time=avg_cpu_time+cpu_time;
        if (cpu_time>max_cpu_time) {max_cpu_time=cpu_time;}
#endif
    }

/*  increase interrupt count */
    icnt++;


    return;
}


/* add ns to timespec */
void rtc_add_timespec(struct timespec *ts, signed long long addtime)
{
	signed long long sec, nsec;
	
	nsec = addtime % NSEC_PER_SEC;
	sec = (addtime - nsec) / NSEC_PER_SEC;
	ts->tv_sec += sec;
	ts->tv_nsec += nsec;
	if ( ts->tv_nsec > NSEC_PER_SEC ) 
	{ 
		nsec = ts->tv_nsec % NSEC_PER_SEC;
		ts->tv_sec += (ts->tv_nsec - nsec) / NSEC_PER_SEC;
		ts->tv_nsec = nsec;
	}	
}	

void rtc_rttask(void *ptr)
{
	struct timespec   ts;
	int rc;
	//int ht;
	signed long long cycletime;
	
	clock_gettime(CLOCK_REALTIME, &ts);

    	/* Convert from timeval to timespec */
	cycletime = *(double*)ptr * NSEC_PER_SEC; /* cycletime in ns */
	while(!istop)
	{	
		/* calculate next cycle start */
		rtc_add_timespec(&ts, cycletime);
		/* wait to cycle start */
		rc = pthread_cond_timedwait(&cond, &mutex, &ts);
		rtc_signalhandler(0);
	}	 

}






int rtc_sleep(int secs)
{
    rtc_usleep(USEC_PER_SEC*secs);

    return 0;
}


int rtc_usleep(int usecs)
{
    unsigned int dusecs;
    double t, te;
    
    t=rtc_get_raw_time();
    
    te=t+((double) usecs)/USEC_PER_SEC_F;
    
    while (t<te) {
	dusecs=(int) ((te-t)*USEC_PER_SEC_F);
	usleep(dusecs);
	t=rtc_get_raw_time();
    }

    return 0;
}


int rtc_timer(double freq, PTIMERCALLBACK user_apc, int priority)
{
  //int iret1;	
  //double t;
  struct sched_param schp;
  int primin, primax;

/* check input timer frequency */
  if (freq<=0.0) {
    printf("rtc_timer (posix ak) reports: invalid timer frequency.\n");
    return -1;
  }

  the_freq=freq;
  the_user_apc=user_apc;

  mlockall(MCL_CURRENT|MCL_FUTURE);
#if 1
/* set priority */
  primin=sched_get_priority_min(SCHED_FIFO);
  primax=sched_get_priority_max(SCHED_FIFO);

  if( ( priority>=primin ) & ( priority<=primax ) ) {
      memset(&schp,0,sizeof(schp));
      schp.sched_priority = priority;

      if(sched_setscheduler(0, SCHED_FIFO, &schp) != 0) {
        printf("rtc_timer (posix ak) reports: error sched_setscheduler.\n");
        usleep(2000000);
        return -1;
      }
      printf("rtc_timer (posix ak) reports: SCHED_FIFO priority set to %d.\n", priority);
  }
#endif

  /*	ignore SIGALRM signals */
  if (priority>0) {
      signal(SIGALRM, SIG_IGN);
      printf("rtc_timer (posix ak) reports: ignoring SIGALRM.\n");
  }

  printf("rtc_timer (posix ak) running at %f Hz.\n", the_freq);

/* 1st second is for timer to get stable */
  istart=10;

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

//  /* create RT thread */
//  iret1 = pthread_create( &thread1, NULL, (void *) &rttask, (void*) &dt);	
//  schp.sched_priority += 1;
//  iret1 = pthread_setschedparam(thread1, SCHED_FIFO, &schp);

  rtc_rttask((void*) &dt);	

//  while (!istop) {
//    rtc_usleep(100000L);
//  }

/* compute average cpu time */
  avg_cpu_time=avg_cpu_time/(icnt-istart-1);
  printf("rtc_timer (posix ak) reports:\n");
  perc_missed=100.0*((double) imissed)/((double) (icnt-istart-1));
  printf("missed interrupts: %6.3f%% [%d out of %d]\n",perc_missed,imissed,(icnt-istart-1));
  printf("average cpu-time used      : %f us\n",avg_cpu_time*USEC_PER_SEC_F);
  printf("average cpu-time used      : %6.1f%%\n",100.0*(avg_cpu_time/dt));
  printf("maximum cpu-time used      : %6.1f%%\n",100.0*(max_cpu_time/dt));

  printf("rtc_timer (posix ak) has been stopped.\n");

  return 0;
}
