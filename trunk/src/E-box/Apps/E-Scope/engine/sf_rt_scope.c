/*
	RT-Scope Simulink scope (Matlab 6 version)

	(c) Rene' van de Molengraft, 2005

	last update: March, 4th, 2005

	Inputs : u[0], u[3]             = adc channels 0 to 3
             u[4], u[7]             = enc channels 0 to 3
	Outputs: y[0], y[3]             = dac channels 0 and 1
*/

#define S_FUNCTION_NAME sf_rt_scope
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#define NSTATES		0
#define NINPUTS		8
#define NOUTPUTS	4
#define NPARAMS		1

#define MAXFREQ		ssGetSFcnParam(S,0)
#define U(element)      (*uPtrs[element])  /* Pointer to Input Port0 */

#include <math.h>

/*====================*
 * S-function methods *
 *====================*/

 



static void mdlInitializeSizes(SimStruct *S)
{
	real_T maxfreq;
	
	ssSetNumSFcnParams(S,NPARAMS);

	ssSetNumContStates(S,NSTATES);
	ssSetNumDiscStates(S,0);

	if (!ssSetNumInputPorts(S,1)) return;
	ssSetInputPortWidth(S,0,NINPUTS);
/*	ssSetInputPortDirectFeedThrough(S,0,NINPUTS);*/
	ssSetInputPortDirectFeedThrough(S,0,0);

	if (!ssSetNumOutputPorts(S,1)) return;
	ssSetOutputPortWidth(S,0,NOUTPUTS);

	ssSetNumSampleTimes(S,1);
	ssSetNumRWork(S,0);
	ssSetNumIWork(S,0);
	ssSetNumPWork(S,0);
	ssSetNumModes(S,0);
	ssSetNumNonsampledZCs(S,0);

#ifndef MATLAB_MEX_FILE
	tde_start_acq_simulink();
#endif
}





static void mdlInitializeSampleTimes(SimStruct *S)
{
	real_T maxfreq;
	
	ssSetSampleTime(S,0,CONTINUOUS_SAMPLE_TIME);
	ssSetOffsetTime(S,0,0.0);
	
#ifndef MATLAB_MEX_FILE
/*	send maxfreq to rtscope */
	maxfreq=1.0/ssGetStepSize(S);
	if (maxfreq<=0.0) {
	    maxfreq=300.0;
	}
	tde_put_maxfreq_simulink(maxfreq);
#endif
}





static void mdlOutputs(SimStruct *S, int_T tid)
{
	real_T *y=ssGetOutputPortRealSignal(S,0);
	InputRealPtrsType uPtrs=ssGetInputPortRealSignalPtrs(S,0);
#ifndef MATLAB_MEX_FILE

	int_T i;
	real_T u[8];

	for (i=0;i<8;i++) {
	    u[i]=U(i);
	}
	
	tde_timer_fun_simulink(&u[0],&y[0]);
#endif
}





static void mdlTerminate(SimStruct *S)
{
}

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
