/*  ec_generic
 * 	
 *  (c)  2011, Michael Ronde, Eindhoven University of Technology
 *
 *  Generic simulink block for variable configuration of EtherCAT slaves
 *
 *  The number of inputs and outputs of a specific type is dependent on the
 *  configuration file "ports.h". This file allocates the correct number of 
 *  ports at build time. At run-time the real number of ports are checked 
 *  against the pre-configured number of ports.
 *
 *  Note: There is a second configuration file "ports.m", which is for the 
 *  visual appearance of the correct numbers of a specific port type in the 
 *  simulink model, changing this has no further influence.
 *      
 *      Inputs:  u0[0] - u0[x]  =   dac channels 0-x
 *               u1[0] - u1[x]  =   do chanels 0-x
 *      Outputs: y1[0] - y1[x]  =   adc channels 0-x
 *               y2[0] - y2[x]  =   encoder channels 0-x
 *               y3[0] - y3[x]  =   di chanels 0-x
 * 
 *      Parameter: 
 *
 *  Versions:
 *  0.2     20110501    -   Moved ec_io() from ec_main.c to ec_generic.c to reduce delay
 *  0.1     20110425    -   Initial version
*/

#define S_FUNCTION_NAME ec_generic
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#define NSTATES         0
#define NPARAMS         0

#define NINPUTS         2
#define NOUTPUTS        3

#define U0(element) (*u0Ptrs[element])  /* Pointer to Input Port0 */
#define U1(element) (*u1Ptrs[element])  /* Pointer to Input Port1 */

#include <math.h>
#include "ports.h"

/*====================*
 * S-function methods *
 *====================*/

static void mdlInitializeSizes(SimStruct *S)
{
	ssSetNumSFcnParams(S,NPARAMS);  /* Number of expected parameters */
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
        return; /* Parameter mismatch will be reported by Simulink */
    }

	ssSetNumContStates(S,NSTATES);
	ssSetNumDiscStates(S,0);

	if (!ssSetNumInputPorts(S,NINPUTS)) return;
	ssSetInputPortWidth(S,0,NDAC);
    ssSetInputPortWidth(S,1,NDOUT);
	/*ssSetInputPortDirectFeedThrough(S,0,0);*/

	if (!ssSetNumOutputPorts(S,NOUTPUTS)) return;
	ssSetOutputPortWidth(S,0,NADC);
    ssSetOutputPortWidth(S,1,NENC);
    ssSetOutputPortWidth(S,2,NDIN);

	ssSetNumSampleTimes(S,1);
	ssSetNumRWork(S,0);
	ssSetNumIWork(S,0);
	ssSetNumPWork(S,0);
	ssSetNumModes(S,0);
	ssSetNumNonsampledZCs(S,0);
}





static void mdlInitializeSampleTimes(SimStruct *S)
{
	ssSetSampleTime(S,0,CONTINUOUS_SAMPLE_TIME);
	ssSetOffsetTime(S,0,0.0);
}





static void mdlOutputs(SimStruct *S, int_T tid)
{
        real_T *yadc    =   ssGetOutputPortRealSignal(S,0);
        real_T *yenc    =   ssGetOutputPortRealSignal(S,1);
        real_T *ydi     =   ssGetOutputPortRealSignal(S,2);
         
        InputRealPtrsType u0Ptrs = ssGetInputPortRealSignalPtrs(S,0);
        InputRealPtrsType u1Ptrs = ssGetInputPortRealSignalPtrs(S,1);

#ifndef MATLAB_MEX_FILE
        int_T ichan, iret, istop;

        if( ssGetT(S) == 0.0){
            printf("Checking configuration\n");
            if(ec_get_nadc() != NADC){
                printf("Configured NADC=%d does not match active NADC=%d \n",NADC,ec_get_nadc());
                istop = 1;
            }
            if(ec_get_ndac() != NDAC){
                printf("Configured NDAC=%d does not match active NDAC=%d \n",NDAC,ec_get_ndac());
                istop = 1;
            }
            if(ec_get_nenc() != NENC){
                printf("Configured NENC=%d does not match active NENC=%d \n",NENC,ec_get_nenc());
                istop = 1;
            }
            if(ec_get_ndin() != NDIN){
                printf("Configured NDIN=%d does not match active NDIN=%d \n",NDIN,ec_get_ndin());
                istop = 1;
            }
            if(ec_get_ndout() != NDOUT){
                printf("Configured NDOUT=%d does not match active NDOUT=%d \n",NDOUT,ec_get_ndout());
                istop = 1;
            }
            if(ec_get_npwm() != NPWM){
                printf("Configured NPWM=%d does not match active NPWM=%d \n",NPWM,ec_get_npwm());
                istop = 1;
            }
            
            if(istop == 1){
                printf("Invalid configuration file, please run \"./portconfig\" \n");
                exit(EXIT_FAILURE);
            } else {
                printf("Configuration OK!\n");
            }
        }

        /* Update mapping before reading the outputs */
        ec_io();

        /* OUTPUTS */
        for (ichan=0; ichan<NADC; ichan++) {
            /* read adc channels */
            ec_adc_read_chan(&yadc[ichan], ichan);
        }
        for (ichan=0; ichan<NENC; ichan++) {
            /* read encoder channels */
            ec_enc_read_chan(&yenc[ichan], ichan);
        }
        
        for (ichan=0; ichan<NDIN; ichan++) {
            /* read digital input channels */
            ec_din_read_chan(&ydi[ichan], ichan);
        }
        
        /* INPUTS*/
        for (ichan=0; ichan<NDAC; ichan++){
            iret = ec_dac_write_chan(U0(ichan), ichan);
            if (iret != 0){
                printf("E-box error \n");
                ec_print_error_msg(iret);
                exit(EXIT_FAILURE);
            }
        }
        for (ichan=0; ichan<NDOUT; ichan++){
            ec_dout_write_chan(U1(ichan), ichan);
        }

        ec_io();


#endif
}



static void mdlTerminate(SimStruct *S)
{
#ifndef MATLAB_MEX_FILE
        int_T ichan;

        for (ichan=0; ichan<NDAC; ichan++){
            ec_dac_write_chan(0.0 , ichan);
        }
        for (ichan=0; ichan<NDOUT; ichan++){
            ec_dout_write_chan(0.0 , ichan);
        }
#endif
}

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
