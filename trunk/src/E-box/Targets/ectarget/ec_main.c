/* $Revision: 1.68.4.6 $
 * Copyright 1994-2004 The MathWorks, Inc.
 *
 * File    : grt_main.c
 *
 * Abstract:
 *      A Generic "Real-Time (single tasking or pseudo-multitasking,
 *      statically allocated data)" main that runs under most
 *      operating systems.
 *
 *      This file may be a useful starting point when targeting a new
 *      processor or microcontroller.
 *
 *
 * Compiler specified defines:
 *	RT              - Required.
 *      MODEL=modelname - Required.
 *	NUMST=#         - Required. Number of sample times.
 *	NCSTATES=#      - Required. Number of continuous states.
 *      TID01EQ=1 or 0  - Optional. Only define to 1 if sample time task
 *                        id's 0 and 1 have equal rates.
 *      MULTITASKING    - Optional. (use MT for a synonym).
 *	SAVEFILE        - Optional (non-quoted) name of .mat file to create.
 *			  Default is <MODEL>.mat
 *      BORLAND         - Required if using Borland C/C++
 */

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtwtypes.h"
#include "rtmodel.h"
#include "rt_sim.h"
#include "rt_logging.h"
#ifdef UseMMIDataLogging
#include "rt_logging_mmi.h"
#endif
#include "rt_nonfinite.h"

/* Signal Handler header */
#ifdef BORLAND
#include <signal.h>
#include <float.h>
#endif

#include "ext_work.h"



/********** ADDED **********/

#include "timer.h"
#include "u1.h"
#include "ec.h"
#include <unistd.h>
#include <pthread.h>
#define DEC ( void *(*)(void*) )

char netif[10];
#ifndef NETIF_ID
#define NETIF_ID	3
#endif

/*=========*
 * Defines *
 *=========*/

#ifndef TRUE
#define FALSE (0)
#define TRUE  (1)
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#endif

#define QUOTE1(name) #name
#define QUOTE(name) QUOTE1(name)    /* need to expand name    */

#ifndef RT
# error "must define RT"
#endif

#ifndef MODEL
# error "must define MODEL"
#endif

#ifndef NUMST
# error "must define number of sample times, NUMST"
#endif

#ifndef NCSTATES
# error "must define NCSTATES"
#endif

#ifndef SAVEFILE
# define MATFILE2(file) #file ".mat"
# define MATFILE1(file) MATFILE2(file)
# define MATFILE MATFILE1(MODEL)
#else
# define MATFILE QUOTE(SAVEFILE)
#endif

#define RUN_FOREVER -1.0

#define EXPAND_CONCAT(name1,name2) name1 ## name2
#define CONCAT(name1,name2) EXPAND_CONCAT(name1,name2)
#define RT_MODEL            CONCAT(MODEL,_rtModel)

/*====================*
 * External functions *
 *====================*/
extern RT_MODEL *MODEL(void);

extern void MdlInitializeSizes(void);
extern void MdlInitializeSampleTimes(void);
extern void MdlStart(void);
extern void MdlOutputs(int_T tid);
extern void MdlUpdate(int_T tid);
extern void MdlTerminate(void);

#if NCSTATES > 0
  extern void rt_ODECreateIntegrationData(RTWSolverInfo *si);
  extern void rt_ODEUpdateContinuousStates(RTWSolverInfo *si);

# define rt_CreateIntegrationData(S) \
    rt_ODECreateIntegrationData(rtmGetRTWSolverInfo(S));
# define rt_UpdateContinuousStates(S) \
    rt_ODEUpdateContinuousStates(rtmGetRTWSolverInfo(S));
# else
# define rt_CreateIntegrationData(S)  \
      rtsiSetSolverName(rtmGetRTWSolverInfo(S),"FixedStepDiscrete");
# define rt_UpdateContinuousStates(S) \
      rtmSetT(S, rtsiGetSolverStopTime(rtmGetRTWSolverInfo(S)));
#endif


/*==================================*
 * Global data local to this module *
 *==================================*/

static struct {
  int_T    stopExecutionFlag;
  int_T    isrOverrun;
  int_T    overrunFlags[NUMST];
  const char_T *errmsg;
} GBLbuf;


#ifdef EXT_MODE
#  define rtExtModeSingleTaskUpload(S)                          \
   {                                                            \
        int stIdx;                                              \
        rtExtModeUploadCheckTrigger(rtmGetNumSampleTimes(S));   \
        for (stIdx=0; stIdx<NUMST; stIdx++) {                   \
            if (rtmIsSampleHit(S, stIdx, 0 /*unused*/)) {       \
                rtExtModeUpload(stIdx,rtmGetTaskTime(S,stIdx)); \
            }                                                   \
        }                                                       \
   }
#else
#  define rtExtModeSingleTaskUpload(S) /* Do nothing */
#endif

/*=================*
 * Local functions *
 *=================*/

#ifdef BORLAND
/* Implemented for BC++ only*/

typedef void (*fptr)(int, int);

/* Function: divideByZero =====================================================
 *
 * Abstract: Traps the error Division by zero and prints a warning
 *           Also catches other FP errors, but does not identify them
 *           specifically.
 */
void divideByZero(int sigName, int sigType)
{
    signal(SIGFPE, (fptr)divideByZero);
    if ((sigType == FPE_ZERODIVIDE)||(sigType == FPE_INTDIV0)){
        printf("*** Warning: Division by zero\n\n");
        return;
    }
    else{
        printf("*** Warning: Floating Point error\n\n");
        return;
    }
} /* end divideByZero */

#endif /* BORLAND */

#if !defined(MULTITASKING)  /* SINGLETASKING */

/* Function: rtOneStep ========================================================
 *
 * Abstract:
 *      Perform one step of the model. This function is modeled such that
 *      it could be called from an interrupt service routine (ISR) with minor
 *      modifications.
 */



/********* MODIFIED **********/

/* changed S to Sarg in rt_OneStep */

/***************************** */



static void rt_OneStep(RT_MODEL *Sarg)
{
    real_T tnext;

    /***********************************************
     * Check and see if base step time is too fast *
     ***********************************************/

    if (GBLbuf.isrOverrun++) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }

    /***********************************************
     * Check and see if error status has been set  *
     ***********************************************/

    if (rtmGetErrorStatus(Sarg) != NULL) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }

    /* enable interrupts here */

    /*
     * In a multi-tasking environment, this would be removed from the base rate
     * and called as a "background" task.
     */



/********** MODIFIED **********/

/*    rtExtModeOneStep(rtmGetRTWExtModeInfo(Sarg),
                     rtmGetNumSampleTimes(Sarg),
                     (boolean_T *)&rtmGetStopRequested(Sarg));

*/



    tnext = rt_SimGetNextSampleHit();
    rtsiSetSolverStopTime(rtmGetRTWSolverInfo(Sarg),tnext);

    MdlOutputs(0);

    rtExtModeSingleTaskUpload(Sarg);

    GBLbuf.errmsg = rt_UpdateTXYLogVars(rtmGetRTWLogInfo(Sarg),
                                        rtmGetTPtr(Sarg));
    if (GBLbuf.errmsg != NULL) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }
    /*edit target 2007b*/
    /*rt_UpdateSigLogVars(rtmGetRTWLogInfo(Sarg), rtmGetTPtr(Sarg));*/

    MdlUpdate(0);
    rt_SimUpdateDiscreteTaskSampleHits(rtmGetNumSampleTimes(Sarg),
                                       rtmGetTimingData(Sarg),
                                       rtmGetSampleHitPtr(Sarg),
                                       rtmGetTPtr(Sarg));

    if (rtmGetSampleTime(Sarg,0) == CONTINUOUS_SAMPLE_TIME) {
        rt_UpdateContinuousStates(Sarg);
    }

    GBLbuf.isrOverrun--;

    rtExtModeCheckEndTrigger();

} /* end rtOneStep */

#else /* MULTITASKING */

# if TID01EQ == 1
#  define FIRST_TID 1
# else
#  define FIRST_TID 0
# endif

/* Function: rtOneStep ========================================================
 *
 * Abstract:
 *      Perform one step of the model. This function is modeled such that
 *      it could be called from an interrupt service routine (ISR) with minor
 *      modifications.
 *
 *      This routine is modeled for use in a multitasking environment and
 *	therefore needs to be fully re-entrant when it is called from an
 *	interrupt service routine.
 *
 * Note:
 *      Error checking is provided which will only be used if this routine
 *      is attached to an interrupt.
 *
 */



/********* MODIFIED **********/

/* changed S to Sarg in rt_OneStep */

/*****************************/
/*edit target 2007b*/
/*rt_FillSigLogInfoFromMMI(rtmGetRTWLogInfo(S),&rtmGetErrorStatus(S));*/

static void rt_OneStep(RT_MODEL *Sarg)
{
    int_T  eventFlags[NUMST];
    int_T  i;
    real_T tnext;
    int_T  *sampleHit = rtmGetSampleHitPtr(Sarg);

    /***********************************************
     * Check and see if base step time is too fast *
     ***********************************************/

    if (GBLbuf.isrOverrun++) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }

    /***********************************************
     * Check and see if error status has been set  *
     ***********************************************/

    if (rtmGetErrorStatus(Sarg) != NULL) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }
    /* enable interrupts here */

    /*
     * In a multi-tasking environment, this would be removed from the base rate
     * and called as a "background" task.
     */



/********** MODIFIED **********/
/*
//    rtExtModeOneStep(rtmGetRTWExtModeInfo(Sarg),
//                     rtmGetNumSampleTimes(Sarg),
//                     (boolean_T *)&rtmGetStopRequested(Sarg));

******************************/



    /************************************************************************
     * Update discrete events and buffer event flags locally so that ISR is *
     * re-entrant.                                                          *
     ************************************************************************/

    tnext = rt_SimUpdateDiscreteEvents(rtmGetNumSampleTimes(Sarg),
                                       rtmGetTimingData(Sarg),
                                       rtmGetSampleHitPtr(Sarg),
                                       rtmGetPerTaskSampleHitsPtr(Sarg));
    rtsiSetSolverStopTime(rtmGetRTWSolverInfo(Sarg),tnext);
    for (i=FIRST_TID+1; i < NUMST; i++) {
        eventFlags[i] = sampleHit[i];
    }

    /*******************************************
     * Step the model for the base sample time *
     *******************************************/

    MdlOutputs(FIRST_TID);

    rtExtModeUploadCheckTrigger(rtmGetNumSampleTimes(Sarg));
    rtExtModeUpload(FIRST_TID,rtmGetTaskTime(Sarg, FIRST_TID));

    GBLbuf.errmsg = rt_UpdateTXYLogVars(rtmGetRTWLogInfo(Sarg),
                                        rtmGetTPtr(Sarg));
    if (GBLbuf.errmsg != NULL) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }

    MdlUpdate(FIRST_TID);

    if (rtmGetSampleTime(Sarg,0) == CONTINUOUS_SAMPLE_TIME) {
        rt_UpdateContinuousStates(Sarg);
    }
     else {
        rt_SimUpdateDiscreteTaskTime(rtmGetTPtr(Sarg), 
                                     rtmGetTimingData(Sarg), 0);
    }

#if FIRST_TID == 1
    rt_SimUpdateDiscreteTaskTime(rtmGetTPtr(Sarg), 
                                 rtmGetTimingData(Sarg),1);
#endif


    /************************************************************************
     * Model step complete for base sample time, now it is okay to          *
     * re-interrupt this ISR.                                               *
     ************************************************************************/

    GBLbuf.isrOverrun--;


    /*********************************************
     * Step the model for any other sample times *
     *********************************************/

    for (i=FIRST_TID+1; i<NUMST; i++) {
        if (eventFlags[i]) {
            if (GBLbuf.overrunFlags[i]++) {  /* Are we sampling too fast for */
                GBLbuf.stopExecutionFlag=1;  /*   sample time "i"?           */
                return;
            }

            MdlOutputs(i);

            rtExtModeUpload(i, rtmGetTaskTime(Sarg,i));

            MdlUpdate(i);
            
            /*edit taget 2007b*/
            /*rt_FillSigLogInfoFromMMI(rtmGetRTWLogInfo(S),&rtmGetErrorStatus(S));*/

            rt_SimUpdateDiscreteTaskTime(rtmGetTPtr(Sarg), 
                                         rtmGetTimingData(Sarg),i);

            /* Indicate task complete for sample time "i" */
            GBLbuf.overrunFlags[i]--;
        }
    }

    rtExtModeCheckEndTrigger();

} /* end rtOneStep */

#endif /* MULTITASKING */


static void displayUsage (void)
{
    (void) printf("usage: %s -tf <finaltime> -w -port <TCPport>\n",QUOTE(MODEL));
    (void) printf("arguments:\n");
    (void) printf("  -tf <finaltime> - overrides final time specified in "
                  "Simulink (inf for no limit).\n");
    (void) printf("  -w              - waits for Simulink to start model "
                  "in External Mode.\n");
    (void) printf("  -port <TCPport> - overrides 17725 default port in "
                  "External Mode, valid range 256 to 65535.\n");
}

/*===================*
 * Visible functions *
 *===================*/



/********** ADDED **********/

/* pointer to global copy of model */
static RT_MODEL *S;

static pthread_attr_t MenuThreadAttr;
static pthread_t MenuThreadID;
static int nDevs=0;
static double StepSize, Freq;
static int StopRTA=0;

#ifdef EXT_MODE
/* External Mode thread ID */
static int StopExtMode=0, ExtModeLeft=0;
static pthread_attr_t ExtModeThreadAttr;
static pthread_t ExtModeThreadID;

void* ExtModeThread(void* pvoid)
{
	while(!StopExtMode) {
		rtExtModeOneStep(rtmGetRTWExtModeInfo(S),rtmGetNumSampleTimes(S),(boolean_T*)&rtmGetStopRequested(S));
		rtc_usleep(100000);
	}
	ExtModeLeft=1;

	return NULL;
}
#endif





int CmdGo(void)
{
    printf("RTA is already running.\n");

    return 0;
}




int CmdHalt(void)
{
    printf("RTA will be stopped now...\n");
    StopRTA=1;

    return 0;
}




int CmdTime(void)
{
    timer_stat stat;
    double perc_missed;
    
    rtc_get_timer_stat(&stat);

    if (stat.icnt>0) {
    	stat.avg_cpu_time=stat.avg_cpu_time/stat.icnt;
    	printf("timer reports:\n");
    	perc_missed=100.0*((double) stat.imissed)/((double) stat.icnt);
    	printf("missed interrupts: %6.3f %% ( %d out of %d )\n",perc_missed,stat.imissed,stat.icnt);
    	printf("average cpu-time used      : %f us\n",stat.avg_cpu_time*1000000.00);
    	printf("average cpu-time used      : %6.1f %%\n",100.0*(stat.avg_cpu_time/stat.dt));
    	printf("maximum cpu-time used      : %6.1f %%\n",100.0*(stat.max_cpu_time/stat.dt));
    }
    
    return 0;
}





void* MenuThread(void* pvoid)
{
    int key;
    
    u1ini("/home/ebox/MATLAB/R2015b/rtw/c/ectarget/ectarget.u1");

    do {
	key=u1key(0);
	switch ( key ) {
	case 10:
	    CmdGo();
	    break;
	case 20:
	    CmdHalt();
	    break;
	case 30:
	    CmdTime();
	    break;
	case 99:
	    break;
	}
/*  } while ( !StopRTA );*/
    } while ( 1 );
    u1fini();
    
    return NULL;
}





static int user_apc(int icnt, int imissed)
{   
    if (!StopRTA && !GBLbuf.stopExecutionFlag &&
           (rtmGetTFinal(S) == RUN_FOREVER ||
            rtmGetTFinal(S)-rtmGetT(S) > rtmGetT(S)*DBL_EPSILON)) {

        rtExtModePauseIfNeeded(rtmGetRTWExtModeInfo(S),
                               rtmGetNumSampleTimes(S),
                               (boolean_T *)&rtmGetStopRequested(S));

        if (rtmGetStopRequested(S)) {
            /* MRonde 20110417: On stop requested keep sending packets */
            ec_io();
            return 1;
        }

        /* ec_io() should only be used in normal case, but not when generic 
         * port structure is used, in that case the s-function block takes 
         * care of ec_io() operation*/
        #ifndef EC_GENERIC
            /* perform I/O first to obtain measurement inputs */
            ec_receive();
            #ifdef DEBUG
                printf("This should not be executed for EC_GENERIC\n");
            #endif
        #endif
        rt_OneStep(S);
        #ifndef EC_GENERIC
            /* perform I/O to make write values effective at output */
            ec_send();
        #endif
    } else {
        /* MRonde 20120115: On stop requested keep sending packets */
        ec_io();
        return 1;
    }

    return 0;
}    

/****************************/



/* Function: main =============================================================
 *
 * Abstract:
 *      Execute model on a generic target such as a workstation.
 */
int_T main(int_T argc, const char_T *argv[])
{



/********** MODIFIED **********/
/*
  model has become global instead
  RT_MODEL  *S;
*/
/******************************/



    const char *status;
    real_T     finaltime = -2.0;

    int_T  oldStyle_argc;
    const char_T *oldStyle_argv[5];

    /******************************
     * MathError Handling for BC++ *
     ******************************/
#ifdef BORLAND
    signal(SIGFPE, (fptr)divideByZero);
#endif

    /*******************
     * Parse arguments *
     *******************/

    if ((argc > 1) && (argv[1][0] != '-')) {
        /* old style */
        if ( argc > 3 ) {
            displayUsage();
            exit(EXIT_FAILURE);
        }

        oldStyle_argc    = 1;
        oldStyle_argv[0] = argv[0];
    
        if (argc >= 2) {
            oldStyle_argc = 3;

            oldStyle_argv[1] = "-tf";
            oldStyle_argv[2] = argv[1];
        }

        if (argc == 3) {
            oldStyle_argc = 5;

            oldStyle_argv[3] = "-port";
            oldStyle_argv[4] = argv[2];

        }

        argc = oldStyle_argc;
        argv = oldStyle_argv;

    }

    {
        /* new style: */
        double    tmpDouble;
        char_T tmpStr2[200];
        int_T  count      = 1;
        int_T  parseError = FALSE;

        /*
         * Parse the standard RTW parameters.  Let all unrecognized parameters
         * pass through to external mode for parsing.  NULL out all args handled
         * so that the external mode parsing can ignore them.
         */
        while(count < argc) {
            const char_T *option = argv[count++];
            
            /* final time */
            if ((strcmp(option, "-tf") == 0) && (count != argc)) {
                const char_T *tfStr = argv[count++];
                
                sscanf(tfStr, "%200s", tmpStr2);
                if (strcmp(tmpStr2, "inf") == 0) {
                    tmpDouble = RUN_FOREVER;
                } else {
                    char_T tmpstr[2];

                    if ( (sscanf(tmpStr2,"%lf%1s", &tmpDouble, tmpstr) != 1) ||
                         (tmpDouble < 0.0) ) {
                        (void)printf("finaltime must be a positive, real value or inf\n");
                        parseError = TRUE;
                        break;
                    }
                }
                finaltime = (real_T) tmpDouble;

                argv[count-2] = NULL;
                argv[count-1] = NULL;
            }
        }

        if (parseError) {
            (void)printf("\nUsage: %s -option1 val1 -option2 val2 -option3 "
                         "...\n\n", QUOTE(MODEL));
            (void)printf("\t-tf 20 - sets final time to 20 seconds\n");

            exit(EXIT_FAILURE);
        }

        rtExtModeParseArgs(argc, argv, NULL);

        /*
         * Check for unprocessed ("unhandled") args.
         */
        {
            int i;
            for (i=1; i<argc; i++) {
                if (argv[i] != NULL) {
                    printf("Unexpected command line argument: %s\n",argv[i]);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }



    /****************************
     * Initialize global memory *
     ****************************/
    (void)memset(&GBLbuf, 0, sizeof(GBLbuf));

    /************************
     * Initialize the model *
     ************************/
    rt_InitInfAndNaN(sizeof(real_T));

    S = MODEL();
    if (rtmGetErrorStatus(S) != NULL) {
        (void)fprintf(stderr,"Error during model registration: %s\n",
                      rtmGetErrorStatus(S));
        exit(EXIT_FAILURE);
    }
    if (finaltime >= 0.0 || finaltime == RUN_FOREVER) rtmSetTFinal(S,finaltime);

    MdlInitializeSizes();
    MdlInitializeSampleTimes();
    
    status = rt_SimInitTimingEngine(rtmGetNumSampleTimes(S),
                                    rtmGetStepSize(S),
                                    rtmGetSampleTimePtr(S),
                                    rtmGetOffsetTimePtr(S),
                                    rtmGetSampleHitPtr(S),
                                    rtmGetSampleTimeTaskIDPtr(S),
                                    rtmGetTStart(S),
                                    &rtmGetSimTimeStep(S),
                                    &rtmGetTimingData(S));

    if (status != NULL) {
        (void)fprintf(stderr,
                "Failed to initialize sample time engine: %s\n", status);
        exit(EXIT_FAILURE);
    }
    rt_CreateIntegrationData(S);

#ifdef UseMMIDataLogging
    rt_FillStateSigInfoFromMMI(rtmGetRTWLogInfo(S),&rtmGetErrorStatus(S));
    /*edit target 2007b*/
    /*rt_FillSigLogInfoFromMMI(rtmGetRTWLogInfo(S),&rtmGetErrorStatus(S));*/
#endif
    GBLbuf.errmsg = rt_StartDataLogging(rtmGetRTWLogInfo(S),
                                        rtmGetTFinal(S),
                                        rtmGetStepSize(S),
                                        &rtmGetErrorStatus(S));
    if (GBLbuf.errmsg != NULL) {
        (void)fprintf(stderr,"Error starting data logging: %s\n",GBLbuf.errmsg);
        return(EXIT_FAILURE);
    }



/********** ADDED **********/

#ifdef EXT_MODE
    printf("Waiting for External Mode start...\n");
#endif

/***************************/



    rtExtModeCheckInit(rtmGetNumSampleTimes(S));
    rtExtModeWaitForStartPkt(rtmGetRTWExtModeInfo(S),
                             rtmGetNumSampleTimes(S),
                             (boolean_T *)&rtmGetStopRequested(S));



/********** MODIFIED **********/
/*
//    (void)printf("\n** starting the model **\n");
*/
/******************************/



    MdlStart();
    if (rtmGetErrorStatus(S) != NULL) {
      GBLbuf.stopExecutionFlag = 1;
    }

    /*************************************************************************
     * Execute the model.  You may attach rtOneStep to an ISR, if so replace *
     * the call to rtOneStep (below) with a call to a background task        *
     * application.                                                          *
     *************************************************************************/

    if (rtmGetTFinal(S) == RUN_FOREVER) {
        printf ("\n**May run forever. Model stop time set to infinity.**\n");
    }



/********** MODIFIED **********/

#ifdef EXT_MODE
/*	start external mode thread */

	pthread_attr_init(&ExtModeThreadAttr);
	pthread_attr_setdetachstate(&ExtModeThreadAttr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&ExtModeThreadID, &ExtModeThreadAttr, DEC  ExtModeThread, NULL)) {
            printf("pthread_create ExtModeThread failed.\n");
            exit(EXIT_FAILURE);
    }
#endif

/*	start menu thread */
    /*
	pthread_attr_init(&MenuThreadAttr);
	pthread_attr_setdetachstate(&MenuThreadAttr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&MenuThreadID, &MenuThreadAttr, DEC  MenuThread, NULL)) {
            printf("pthread_create MenuThread failed.\n");
            exit(EXIT_FAILURE);
    }
     */

/*    while (!GBLbuf.stopExecutionFlag &&
//           (rtmGetTFinal(S) == RUN_FOREVER ||
//            rtmGetTFinal(S)-rtmGetT(S) > rtmGetT(S)*DBL_EPSILON)) {

//        rtExtModePauseIfNeeded(rtmGetRTWExtModeInfo(S),
//                               rtmGetNumSampleTimes(S),
//                               (boolean_T *)&rtmGetStopRequested(S));

//    if (rtmGetStopRequested(S)) break;
//        rt_OneStep(S);
//    }
*/


/********** ADDED **********/

/* start timer */
    printf("\n");
    printf("ECtarget v1.4 SOEM \n");
    printf("M.J.G. v.d. Molengraft, Eindhoven University of Technology, 2004-2011 \n");
    printf("M.J.C. Ronde, Eindhoven University of Technology, 2010-2011 \n");
    printf("IME Technologies, 2008 \n\n");

    StepSize=rtmGetStepSize(S);
    Freq=1/StepSize;
    
    strcpy(netif, "eth");
    *(netif+3)=(char) (48+NETIF_ID);
    *(netif+4)='\0';
    printf("Net interface = %s.\n", netif);
    
    if (ec_start(netif) != EC_SUCCESS){
        printf("EtherCAT initialization failed.\n");
        return(EXIT_FAILURE);
    }

    rtc_timer(Freq,user_apc,90); /*startup and execution Real-Time process*/
     
    if (!GBLbuf.stopExecutionFlag && !rtmGetStopRequested(S)) {
        /* MRonde: 20110417 send data before rt_OneStep(S) similar to user_apc */
        ec_io();
        /* Execute model last time step */
        rt_OneStep(S);
    }


#ifdef EXT_MODE
    /*  stop external mode thread */
    StopExtMode=1;
    while (!ExtModeLeft) {
        printf("Waiting for External Mode to end...\n");
        /* MRonde 20110417 changed rtc_usleep to 50000 and added ec_io() to prevent error on shutdown */
        ec_io();
        /* normal (u)sleep doesn't seem to work under posix timer due to SIGNAL */
        rtc_usleep(50000);
    }
#endif
    printf("External Mode has ended.\n");
    /* MRonde 20120115 Do ec_stop before writing mat file */
    ec_io();
    ec_stop();

    /********************
     * Cleanup and exit *
     ********************/
#ifdef UseMMIDataLogging
    rt_CleanUpForStateLogWithMMI(rtmGetRTWLogInfo(S));
    /*edit target 2007b*/
    /* rt_CleanUpForSigLogWithMMI(rtmGetRTWLogInfo(S));*/
#endif
    rt_StopDataLogging(MATFILE,rtmGetRTWLogInfo(S));

    rtExtModeShutdown(rtmGetNumSampleTimes(S));

    if (GBLbuf.errmsg) {
        (void)fprintf(stderr,"%s\n",GBLbuf.errmsg);
        exit(EXIT_FAILURE);
    }

    if (GBLbuf.isrOverrun) {
        (void)fprintf(stderr,
                      "%s: ISR overrun - base sampling rate is too fast\n",
                      QUOTE(MODEL));
        exit(EXIT_FAILURE);
    }

    if (rtmGetErrorStatus(S) != NULL) {
        (void)fprintf(stderr,"%s\n", rtmGetErrorStatus(S));
        exit(EXIT_FAILURE);
    }
#ifdef MULTITASKING
    else {
        int_T i;
        for (i=1; i<NUMST; i++) {
            if (GBLbuf.overrunFlags[i]) {
                (void)fprintf(stderr,
                        "%s ISR overrun - sampling rate is too fast for "
                        "sample time index %d\n", QUOTE(MODEL), i);
                exit(EXIT_FAILURE);
            }
        }
    }
#endif

    MdlTerminate();
    

    return(EXIT_SUCCESS);

} /* end main */



/* EOF: grt_main.c */
