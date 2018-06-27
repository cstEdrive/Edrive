//#define DEBUG
//#define DUMMY
//#define LOG1

/* Scope engine
 *
 * Ren??? van de Molengraft, November, 2001
 * revision history: April, 16th, 2003
 *					 June, 21th, 2003: linked to new TUeDACS interface library
 *					 June, 27th, 2003: bug fixes
 *					 July, 2nd, 2003: triggering based on data history
 *					 November, 24th, 2003: MAXFREQ macro can be easily redefined
 *					 March, 7th, 2004: added enhanced trigger signal filtering
 *					 June, 30th, 2004: linux port
 *					 July, 28th, 2004: separated tdext from tde_gate via shared memory ipc
 *					 October, 24th, 2004: maxfreq enters tdext as argument in tde_start_acq
 *					 June, 2005: implemented tcp/ip IPC
 *					 October, 2005: adapted for USB
 *					 October, 9th, 2005: software encoder reset
 *                                       January, 21th, 2011: adapted for use with E-Box
 * Michael Ronde, May, 3rd, 2011: adapted for use with E-box
 *                May, 11th, 2011: added ec_io() to timer_fun
 *                May, 29th, 2011: adapted timer_fun for logical port api
 *                                 cleanup and comments (added prototypes, removed unused variables
 *		  August,31th,2011: Added logging possibility and quick solution to return first device as E/BOX
 * 		  September, 4th, Added funtionality to get number of adc, dac, enc, pwn, din and dout
 *        September,28th, Added posibility for dummy data
 *        September,30th, Fixed funtionalty to get adc and enc state 
 *
 * Rene van de Molengraft, February, 3rd, 2015: Fixed qlgui offset slider.
 * 
 * TODO:
 * - change for generic EtherCAT stack 
 * - timer_fun RW order in case of downsampling 
 */


#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>

#include <fungen.h>
#include <../fir/fir.h>
#include <../incomm/incomm.h>
#include <../div/div.h>
#include <string.h>
#include "timer.h"
#include "engine.h"

#ifndef _EC_TYPE_H
    #include "/usr/src/soem/ethercattype.h"
#endif    

#include <ec.h>

#define DEC	( void *(*)(void*) )


/* Number of ports per type */
static int tde_ndevs;           /* Obsolete ?*/
static int tde_nadc;
static int tde_ndac;
static int tde_nenc;

static int tde_started;
/* MAXCHAN adc and MAXCHAN enc inputs are supported */
static double tde_data[MAXSAMP*MAXCHAN*2];
static int tde_nsamp;       /* number of samples/frame */
static int tde_buffer_idx;  /* index for data buffer */
static int tde_frame_idx;
static int tde_data_idx;
static int tde_go;
static int tde_valid;   /* 1 if data is valid, else 0 */
static int tde_read;    /* 1 if data is read from buffer, else 0 */
static int tde_ndown;   /* downsampling factor of ADC and ENC */               
static int tde_idown;   /* counter for ignored values due to downsamping */
static int tde_trigger;
static int tde_triggered;
static int tde_trig_signal;
static int tde_trig_type;
static int tde_trig_state;
static int tde_clip[MAXCHAN];   /* adc clipped ?*/
static int tde_linked;
static double tde_adc_offset[MAXCHAN];
// static double tde_freq;
static double tde_trig_level;
static double tde_trig_preroll;
static double tde_trig_val_hist[MAXHIST];
static int tde_ntaps;
// static int tde_timer_stopped;
static double tde_t0;
static double tde_maxfreq;
static int tde_dac_ndown;   /* downsampling factor of DAC */ 
static int tde_dac_idown;   /* counter for ignored values due to downsamping */ 
static int tde_is_simulink; /* 1 if running from simulink */
static int tde_running;
static double tde_enc0[MAXCHAN];
static double tde_last_enc[MAXCHAN];

/* States used for simulink purposes */
static int tde_adc_state[MAXCHAN];
static int tde_enc_state[MAXCHAN];
static int tde_dac_state[MAXCHAN];

// user command
static tde_cmd cmd;

static int run=1;

static tde_shm copy_of_shm;
static double copy_of_data[MAXSAMP*MAXCHAN*2];

#ifdef LOG1
    #define MAXLOG1 1000000
    /* Pointer for log files*/
    static FILE *tde_log1_fp;
    static double tde_log1_data[MAXLOG1];
    static double tde_log1_data2[MAXLOG1];    
    static double tde_log1_time[MAXLOG1];
#endif





//void _init()
//{
//}





//void _fini()
//{
//}





double absval(double x)
{
	if (x>=0) {
		return x;
	} else {
		return -x;
	}
}





double signval(double x)
{
	if (x>=0) {
		return 1;
	} else {
		return -1;
	}
}





int igetmax(int x,int y)
{
	if (x>=y) {
		return x;
	} else {
		return y;
	}
}


int tde_reset_time(void)
{
  
  tde_t0=rtc_get_nominal_time();

  return 0;
}





double tde_get_time(void)
{ 
  double t;
  
  t=rtc_get_nominal_time()-tde_t0;
  
  return t;
}




/* prototype
int tde_timer_fun(int istep, int imissed);
 */


int clear_user_command(void)
{
    int i;
    
/*  Clear user cmd  */
    cmd.status=0;
    cmd.delayed=0;
    cmd.id=0;
    cmd.nargs=0;
    for (i=0;i<6;i++) {
	cmd.parg[i]=NULL;
    }
        
    return 0;
}





void* shm_thread(void* cookie)
{
    struct sched_param threadschedparam;
    int retval;
    
    /* set thread priority */
    memset(&threadschedparam,0,sizeof(threadschedparam));
    threadschedparam.sched_priority=20;
    retval = sched_setscheduler(0,SCHED_FIFO,&threadschedparam);
#ifdef DEBUG    
    printf("Return value= %d (%d %d %d) \n",retval,EINVAL,EPERM,ESRCH);
#endif

    if ( incomm_server_open(2039)==INCOMM_ERR ) {
        printf("incomm_server_open failed\n");
        tde_stop_acq();
        return NULL;
    }

    while (run) {
	if ( incomm_server_read((void*) &copy_of_shm, sizeof(tde_shm))==INCOMM_ERR ) {
	    printf("read_data failed.\n");
	    return NULL;
	}

//	printf("command %d has arrived!\n",copy_of_shm.cmd_id);

	cmd.id=copy_of_shm.cmd_id;

	switch (cmd.id)
	{
		case CALL_TDE_STOP_ACQ:
		case CALL_TDE_CHECK_DATA:
		case CALL_TDE_SAY_READY:
		case CALL_TDE_TRIGGER_OFF:
		case CALL_TDE_LINK_IS_ACTIVE:
		case CALL_FG_SET_DEFAULTS:
			cmd.nargs=0;
			break;
		case CALL_TDE_GET_DATA:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_data[0]);
			cmd.parg[1]=&(copy_of_shm.argin1);
			break;
		case CALL_TDE_GET_INFO:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_shm.maxchan);
			cmd.parg[1]=&(copy_of_shm.maxframe);
			break;
		case CALL_TDE_TRIGGER_ON:
			cmd.nargs=6;
			cmd.parg[0]=&(copy_of_shm.argin1);
			cmd.parg[1]=&(copy_of_shm.argin2);
			cmd.parg[2]=&(copy_of_shm.argin3);
			cmd.parg[3]=&(copy_of_shm.argin4);		
			cmd.parg[4]=&(copy_of_shm.argin5);
			cmd.parg[5]=&(copy_of_shm.argin6);
			break;
		case CALL_TDE_CHECK_CLIPPING:
			cmd.nargs=1;
			cmd.parg[0]=&(copy_of_shm.clip[0]);
			break;
		case CALL_TDE_SET_ADC_STATE:
		case CALL_TDE_SET_ENC_STATE:
		case CALL_TDE_SET_DAC_STATE:
		case CALL_TDE_SET_ADC_OFFSET:
		case CALL_TD_GET_RANGE:
		case CALL_TD_GET_FILTER_ENABLE:
		case CALL_FG_GET_VAL:
		case CALL_FG_SET_AMP1:
		case CALL_FG_SET_AMP2:
		case CALL_FG_SET_FREQ1:
		case CALL_FG_SET_FREQ2:
		case CALL_FG_SET_OFFSET:
		case CALL_FG_SET_TYPE:
		case CALL_FG_SET_HOLD_ORDER:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_shm.argin1);
			cmd.parg[1]=&(copy_of_shm.argin2);
			break;
		case CALL_TDE_GET_ADC_STATE:
		case CALL_TDE_GET_ENC_STATE:
		case CALL_TDE_GET_DAC_STATE:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_shm.stat);
			cmd.parg[1]=&(copy_of_shm.argin1);
			break;
		case CALL_TDE_GET_ADC_OFFSET:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_shm.offset);
			cmd.parg[1]=&(copy_of_shm.argin1);
			break;
		case CALL_TDE_PAUSE:
		case CALL_TDE_SET_NSAMP:
		case CALL_TDE_SET_NDOWN:
		case CALL_TDE_INIT_ENC:
		case CALL_TD_GET_DEVICE_TYPE:
		case CALL_TD_AAFILTER_OFF:
		case CALL_TDE_LINK2MEAS:
		case CALL_FG_START:
		case CALL_FG_STOP:
		case CALL_FG_STATUS:
		case CALL_FG_SET_ONOFF:
			cmd.nargs=1;
			cmd.parg[0]=&(copy_of_shm.argin1);
			break;
		case CALL_TD_AAFILTER_ON:
		case CALL_TD_SET_RANGE:
			cmd.nargs=3;
			cmd.parg[0]=&(copy_of_shm.argin1);
			cmd.parg[1]=&(copy_of_shm.argin2);
			cmd.parg[2]=&(copy_of_shm.argin3);
			break;
		case CALL_TDE_GET_MAXFREQ:
			cmd.nargs=1;
			cmd.parg[0]=&(copy_of_shm.maxfreq);
			break;
		case CALL_FG_GET_AMP1:
		case CALL_FG_GET_AMP2:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_shm.amp);
			cmd.parg[1]=&(copy_of_shm.argin1);
			break;
		case CALL_FG_GET_FREQ1:
		case CALL_FG_GET_FREQ2:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_shm.freq);
			cmd.parg[1]=&(copy_of_shm.argin1);
			break;
		case CALL_FG_GET_OFFSET:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_shm.offs);
			cmd.parg[1]=&(copy_of_shm.argin1);
			break;
		case CALL_FG_GET_TYPE:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_shm.type);
			cmd.parg[1]=&(copy_of_shm.argin1);
			break;
		case CALL_FG_GET_ONOFF:
			cmd.nargs=1;
			cmd.parg[0]=&(copy_of_shm.onoff);
			break;
		case CALL_FG_GET_HOLD_ORDER:
			cmd.nargs=2;
			cmd.parg[0]=&(copy_of_shm.holdorder);
			cmd.parg[1]=&(copy_of_shm.argin1);
			break;
		case CALL_TDE_GET_ADC_ENC_STATE:
			cmd.nargs=1;
			cmd.parg[0]=&(copy_of_shm.allstat[0]);
			break;
        case CALL_EE_GET_NADC:  
        case CALL_EE_GET_NDAC:
        case CALL_EE_GET_NENC:
        case CALL_EE_GET_NPWM:
        case CALL_EE_GET_NDIN:
        case CALL_EE_GET_NDOUT:
			cmd.nargs=1;
			cmd.parg[0]=&(copy_of_shm.argin1);
			break;            
		default:
			printf("Unknown command: %d\n",cmd.id);
			break;
	}

//	update status
	cmd.status=copy_of_shm.cmd_status;
	
	switch (cmd.id)
	{
#ifndef USB
	case CALL_TDE_INIT_ENC:
	case CALL_TD_AAFILTER_ON:
	case CALL_TD_AAFILTER_OFF:
	case CALL_TD_SET_RANGE:
	case CALL_FG_SET_HOLD_ORDER:

//	    wait for asynchronous command to finish
	    cmd.delayed=1;
	    while (cmd.delayed==1) {
            rtc_usleep(1000);
	    }

	    break;
#endif
	case CALL_TDE_STOP_ACQ:
	    if (!tde_is_simulink) {
		perform_user_command_direct();
		break;
	    } else {
		break;
	    }
	default:
	    perform_user_command_direct();
	    break;
	}

//	get return value
	copy_of_shm.retval=cmd.retval;
//	printf("Give ready signal\n");
	copy_of_shm.cmd_status=2;
	
	if ( incomm_server_write((void*) &copy_of_shm, sizeof(tde_shm)) == INCOMM_ERR) {
	    printf("write_data failed.\n");
	}

	if (cmd.id==CALL_TDE_GET_DATA) {
//		write data
		if ( incomm_server_write((void*) cmd.parg[0], *cmd.parg[1]*sizeof(double)) == INCOMM_ERR) {
		    printf("write_data failed.\n");
		}
	}

//	if (cmd.id==CALL_TDE_STOP_ACQ) {
//		wait for client to hang up...
//		printf("Waiting for client to hang up...\n");
//		incomm_server_read((void*) &ittt, sizeof(int));
//		printf("Hung up.\n");
//	}
	
//	close socket, but not the listening one!
	rtc_usleep(1000);
//	if (cmd.id==CALL_TDE_STOP_ACQ) {rtc_sleep(2);}
	incomm_server_close2();
	
//	clear cmd struct
	clear_user_command();
    }	

    incomm_server_close();

    return NULL;	
}





int perform_user_command_direct(void)
{
    switch (cmd.id) {
    case CALL_TDE_STOP_ACQ:
	cmd.retval=(double) tde_stop_acq();
	break;
    case CALL_TDE_CHECK_DATA:
	cmd.retval=tde_check_data();
	break;
    case CALL_TDE_SAY_READY:
	cmd.retval=tde_say_ready();
	break;
    case CALL_TDE_GET_DATA:
	cmd.retval=tde_get_data(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_GET_INFO:
	cmd.retval=tde_get_info(cmd.parg[0], cmd.parg[1]);
	break;
    case CALL_TDE_TRIGGER_ON:
	cmd.retval=tde_trigger_on((int) *cmd.parg[0],(int) *cmd.parg[1], *cmd.parg[2], *cmd.parg[3], *cmd.parg[4], (int) *cmd.parg[5]);
	break;
    case CALL_TDE_TRIGGER_OFF:
	cmd.retval=tde_trigger_off();
	break;
    case CALL_TDE_CHECK_CLIPPING:
	cmd.retval=tde_check_clipping(cmd.parg[0]);
	break;
    case CALL_TDE_SET_ADC_STATE:
	cmd.retval=tde_set_adc_state((int) *cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_GET_ADC_STATE:
	cmd.retval=tde_get_adc_state(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_SET_ENC_STATE:
	cmd.retval=tde_set_enc_state((int) *cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_GET_ENC_STATE:
	cmd.retval=tde_get_enc_state(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_SET_DAC_STATE:
	cmd.retval=tde_set_dac_state((int) *cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_GET_DAC_STATE:
	cmd.retval=tde_get_dac_state(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_SET_ADC_OFFSET:
	cmd.retval=tde_set_adc_offset(*cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_GET_ADC_OFFSET:
	cmd.retval=tde_get_adc_offset(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_PAUSE:
	cmd.retval=tde_pause((int) *cmd.parg[0]);
	break;
    case CALL_TDE_SET_NSAMP:
	cmd.retval=tde_set_nsamp((int) *cmd.parg[0]);
	break;
    case CALL_TDE_SET_NDOWN:
	cmd.retval=tde_set_ndown((int) *cmd.parg[0]);
	break;
    case CALL_TD_GET_DEVICE_TYPE:
	/* replace cmd.retval=(double) tde_get_device_type((int) *cmd.parg[0]); */
	/* Return E-box as first and only device */
        if (*cmd.parg[0] == 0){
            cmd.retval=1;
        } else {
            cmd.retval=0;
        }
	break;
    case CALL_TD_GET_RANGE:
	cmd.retval=tde_get_range((int) *cmd.parg[0], (int) *cmd.parg[1]); 
	break;
    case CALL_TD_GET_FILTER_ENABLE:
	cmd.retval=tde_get_filter_enable((int) *cmd.parg[0], (int) *cmd.parg[1]); 
	break;
    case CALL_TDE_GET_MAXFREQ:
	cmd.retval=tde_get_maxfreq(cmd.parg[0]);
	break;
    case CALL_TDE_LINK2MEAS:
	cmd.retval=tde_link2meas((int) *cmd.parg[0]); 
	break;
    case CALL_TDE_LINK_IS_ACTIVE:
	cmd.retval=(double) tde_link_is_active(); 
	break;
    case CALL_FG_GET_VAL:
	cmd.retval=fg_get_val(*cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_START:
	cmd.retval=fg_start((int) *cmd.parg[0]);
	break;
    case CALL_FG_STOP:
	cmd.retval=fg_stop((int) *cmd.parg[0]);
	break;
    case CALL_FG_SET_DEFAULTS:
	cmd.retval=fg_set_defaults();
	break;
    case CALL_FG_SET_AMP1:
	cmd.retval=fg_set_amp1(*cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_GET_AMP1:
	cmd.retval=fg_get_amp1(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_SET_AMP2:
	cmd.retval=fg_set_amp2(*cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_GET_AMP2:
	cmd.retval=fg_get_amp2(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_SET_FREQ1:
	cmd.retval=fg_set_freq1(*cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_GET_FREQ1:
	cmd.retval=fg_get_freq1(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_SET_FREQ2:
	cmd.retval=fg_set_freq2(*cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_GET_FREQ2:
	cmd.retval=fg_get_freq2(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_SET_OFFSET:
	cmd.retval=fg_set_offset(*cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_GET_OFFSET:
	cmd.retval=fg_get_offset(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_STATUS:
	cmd.retval=(double) fg_status((int) *cmd.parg[0]);
	break;
    case CALL_FG_SET_TYPE:
	cmd.retval=fg_set_type((int) *cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_GET_TYPE:
	cmd.retval=fg_get_type(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_FG_SET_ONOFF:
	cmd.retval=fg_set_onoff((int) *cmd.parg[0]);
	break;
    case CALL_FG_GET_ONOFF:
	cmd.retval=fg_get_onoff(cmd.parg[0]);
	break;
    case CALL_FG_GET_HOLD_ORDER:
	cmd.retval=fg_get_hold_order(cmd.parg[0], (int) *cmd.parg[1]);
	break;
    case CALL_TDE_GET_ADC_ENC_STATE:
	cmd.retval=tde_get_adc_enc_state(cmd.parg[0]);
	break;
    /* New E-Scope Enigine Funtionality */
    /* Return globals or get from libec */
    case CALL_EE_GET_NADC:       
        cmd.retval=ec_get_nadc();
    break;
    case CALL_EE_GET_NDAC:       
        cmd.retval=ec_get_ndac();
    break;
    case CALL_EE_GET_NENC:       
        cmd.retval=ec_get_nenc();
    break;
    case CALL_EE_GET_NPWM:
        cmd.retval=ec_get_npwm();
    break;
        case CALL_EE_GET_NDIN:       
        cmd.retval=ec_get_ndin();
    break;
        case CALL_EE_GET_NDOUT:       
        cmd.retval=ec_get_ndout();
    break;
#ifdef USB
    case CALL_TDE_INIT_ENC:
	cmd.retval=tde_init_enc((int) *cmd.parg[0]);
	break;
    case CALL_TD_AAFILTER_ON:
	cmd.retval=tde_aafilter_on(*cmd.parg[0],(int) *cmd.parg[1], (int) *cmd.parg[2]); 
	break;
    case CALL_TD_AAFILTER_OFF:
	cmd.retval=tde_aafilter_off((int) *cmd.parg[0]); 
	break;
    case CALL_TD_SET_RANGE:
	cmd.retval=tde_set_range((int) *cmd.parg[0], (int) *cmd.parg[1], (int) *cmd.parg[2]); 
	break;
    case CALL_FG_SET_HOLD_ORDER:
	cmd.retval=fg_set_hold_order((int) *cmd.parg[0], (int) *cmd.parg[1]);
	break;
#endif
    default:
	cmd.retval=0.0;
	break;
    }

    return 0;
}





int perform_user_command_delayed(void)
{
    if (cmd.delayed!=1) {
//    	nothing to do
	return 0;
    }

#ifndef USB
    switch (cmd.id)
    {
    case CALL_TDE_INIT_ENC:
	cmd.retval=tde_init_enc((int) *cmd.parg[0]);
	break;
    case CALL_TD_AAFILTER_ON:
	cmd.retval=tde_aafilter_on(*cmd.parg[0],(int) *cmd.parg[1], (int) *cmd.parg[2]); 
	break;
    case CALL_TD_AAFILTER_OFF:
	cmd.retval=tde_aafilter_off((int) *cmd.parg[0]); 
	break;
    case CALL_TD_SET_RANGE:
	cmd.retval=tde_set_range((int) *cmd.parg[0], (int) *cmd.parg[1], (int) *cmd.parg[2]); 
	break;
    case CALL_FG_SET_HOLD_ORDER:
	cmd.retval=fg_set_hold_order((int) *cmd.parg[0], (int) *cmd.parg[1]);
	break;
    default:
	cmd.retval=0.0;
	break;
    }
#endif
    
//  command ready
    cmd.delayed=0;

    return 0;
}





int tde_start_engine(double maxfreq, int nsamp, int dac_ndown, char* netif)
{   
#ifdef LOG1    
    int ilog1;
    
    tde_log1_fp = fopen("log1.txt","w");
    if(tde_log1_fp == NULL){
        printf("tde_log reports: Opening log file failed \n");
        return -1;
    }
#endif    
    
//	initialize E-Box devices
    if (ec_start(netif) != 0){
        printf("tde_start_engine reports: no EtherCAT devices connected?\n");
        return EBOX_NONE_FOUND;
    }
    

    /* MRonde: Initialize number of adc, dac and enc */
    tde_nadc = ec_get_nadc();
    tde_ndac = ec_get_ndac();
    tde_nenc = ec_get_nenc();
    /* MRonde: Changed for logical port api */
	if( (tde_nadc+tde_nenc) < 1 && tde_ndac < 1 ){
        printf("tde_start_engine reports: at least one input (adc or enc) and output (dac) channel is required\n");
        return -1;
    }  

#ifdef DEBUG
    printf("tde_start_engine reports: nadc=%d, ndac=%d, nenc=%d\n",tde_nadc,tde_ndac,tde_nenc);
#endif

//	start timer
	rtc_timer(tde_maxfreq, tde_timer_fun,75);

	printf("tde_start_engine reports: timer has stopped.\n");

//	close E-Box devices
	ec_stop();	

//	stop shm server
//	wait some time
	rtc_sleep(2);

#ifdef LOG1    
    /* Write buffer and close log */
    for(ilog1 = 0; ilog1 < MAXLOG1; ilog1++){
        fprintf(tde_log1_fp,"%.15e %.15e %.15e\n",tde_log1_time[ilog1],tde_log1_data[ilog1],tde_log1_data2[ilog1]);
    }        
    fclose(tde_log1_fp);
#endif

	return 0;
}





int tde_start_acq(double maxfreq, int nsamp, int dac_ndown, char* netif)
{
	pthread_t t1;
// 	pthread_attr_t attr1;
 	struct sched_param threadgetparam;
 	int policy;
    int i;
// 	int iret,ichan,ilink;
// 	int nint0=0,nint1=0;
// 	int err;
	
//	engine not yet running
	tde_running=0;

//	standalone scope
	tde_is_simulink=0;
	fg_put_issim(0);

//	store maximum frequency
	tde_maxfreq=maxfreq;
	
//	send maxfreq to function generator also
	fg_put_maxfreq(maxfreq);

//	store dac down
	tde_dac_ndown=dac_ndown;

	tde_go=0;
	tde_linked=0;
	fg_notify(0);
	
	tde_buffer_idx=0;
	tde_frame_idx=0;
	tde_data_idx=-1;
	tde_valid=0;
	tde_read=0;
	tde_started=0;
	tde_idown=0;
	tde_dac_idown=0;
	
	if (nsamp<=MAXSAMP) {
		tde_nsamp=nsamp;
	} else {
		printf("tde_start_acq: maximum number of samples is %d.\n",MAXSAMP);
		tde_nsamp=MAXSAMP;
	}

//	clear buffer
	for (i=0;i<MAXSAMP*MAXCHAN*2;i++) {
		tde_data[i]=0.0;
	}

	for (i=0;i<MAXCHAN;i++) {
		tde_clip[i]=0;
		tde_enc0[i]=0;
		tde_last_enc[i]=0;
	}

//	clear command structure
	clear_user_command();

//	start server thread
//	pthread_attr_init(&attr1);
//	pthread_attr_setscope(&attr1,PTHREAD_SCOPE_PROCESS);
//	pthread_attr_setinheritsched(&attr1,PTHREAD_EXPLICIT_SCHED);
//	pthread_attr_setschedpolicy(&attr1,SCHED_FIFO);
//	memset(&threadschedparam,0,sizeof(threadschedparam));
//	threadschedparam.sched_priority=sched_get_priority_max(SCHED_FIFO);
//	if (pthread_attr_setschedparam(&attr1,&threadschedparam)) {
//	    printf("Cannot set thread priority.");
//	}
	
//	if (pthread_create(&t1, &attr1, DEC shm_thread, NULL)) {
	if (pthread_create(&t1, NULL, DEC shm_thread, NULL)) {
          printf("pthread_create shm_thread failed.\n");
          return -1;
	}

	pthread_getschedparam(t1,&policy,&threadgetparam); 
	printf("Thread priority was set to policy=%d, priority=%d\n",policy, threadgetparam.__sched_priority);

	tde_start_engine(maxfreq, nsamp, dac_ndown, netif);

	return 0;
}





int tde_put_maxfreq_simulink(double maxfreq)
{
        tde_maxfreq=maxfreq;
    
//      send maxfreq to function generator also
        fg_put_maxfreq(maxfreq);

        return 0;
}





int tde_start_acq_simulink(void)
{
	pthread_t t1;
//	pthread_attr_t attr1;
	int i,nsamp;
//  int iret, err;
	
//	simulink rtscope
	tde_is_simulink=1;
	fg_put_issim(1);
	
//	start value for nsamp
	nsamp=256;

//	store dac down
	tde_dac_ndown=0;

	tde_go=0;
	tde_linked=0;
	fg_notify(0);
	
	tde_buffer_idx=0;
	tde_frame_idx=0;
	tde_data_idx=-1;
	tde_valid=0;
	tde_read=0;
	tde_started=0;
	tde_idown=0;
	tde_dac_idown=0;
	
	if (nsamp<=MAXSAMP) {
		tde_nsamp=nsamp;
	} else {
		printf("tde_start_acq: maximum number of samples is %d.\n",MAXSAMP);
		tde_nsamp=MAXSAMP;
	}

//	clear buffer
	for (i=0;i<MAXSAMP*MAXCHAN*2;i++) {
		tde_data[i]=0.0;
	}

	for (i=0;i<MAXCHAN;i++) {
		tde_clip[i]=0;
	}

//	clear command structure
	clear_user_command();

//	start server thread
	if (pthread_create(&t1, NULL, DEC shm_thread, NULL)) {
          printf("pthread_create shm_thread failed.\n");
          return -1;
	 }

//	assume two devices
	tde_ndevs=2;

	return 0;
}





int tde_stop_acq_simulink(void)
{
//	stop shm server
	run=0;
	rtc_sleep(2);
	
	return 0;
}





int tde_pause(int ipause)
{
	int i;
	
	tde_go=1-ipause;
	i=(tde_go==1) & (tde_linked==1);
	fg_notify(i);
	
	if (tde_go==1) {
		tde_buffer_idx=0;
		tde_frame_idx=0;
		tde_data_idx=-1;
		tde_valid=0;
		tde_read=0;
		tde_started=0;
		tde_idown=0;
	}

//	printf("tde_pause: tde_go = %d.\n",tde_go);	
	
	return 0;
}





int tde_set_nsamp(int nsamp)
{
	double obsiv;
	
	tde_buffer_idx=0;
	tde_frame_idx=0;
	tde_data_idx=-1;
	tde_valid=0;
	tde_read=0;

	tde_nsamp=nsamp;

//	(re)calculate obsiv
	obsiv=((double) tde_nsamp)/(tde_maxfreq/(((double) tde_ndown)+1.0));

//	send obsiv to function generator also
	fg_put_obsiv(obsiv);


	return 0;
}





int tde_get_nsamp(void)
{
        return tde_nsamp;
}





int tde_set_ndown(int ndown)
{
	double obsiv;
	
	tde_idown=0;
	tde_ndown=ndown;

//	(re)calculate obsiv
	obsiv=((double) tde_nsamp)/(tde_maxfreq/(((double) tde_ndown)+1.0));

//	send obsiv to function generator also
	fg_put_obsiv(obsiv);

	return 0;
}





int tde_stop_acq(void)
{
//	stop data acquisition engine
	tde_go=-1;
	fg_notify(0);
	
//	stop shm server as well!
	run=0;
	
	return 0;
}





double tde_get_slope(double *pval_hist)
{
	int i,ndata,norder;
	double **xdata,**ydata,**coeff,**pd,res;
	
	ndata=MAXHIST;
	xdata=create_double_matrix(ndata,1);
	ydata=create_double_matrix(ndata,1);

//	points are in opposite order, so result will also have opposite sign...
	for (i=0;i<ndata;i++) {
		xdata[i][0]=(double) i;
		ydata[i][0]=pval_hist[i];
	}

//	fit line
	norder=1;

	coeff=create_double_matrix(norder+1,1);
	polyfit(coeff,xdata,ydata,ndata,norder);

	pd=create_double_matrix(norder+1,1);
	polyget(pd,0.0,coeff,norder);
	res=pd[1][0];

//	free allocated memory
	destroy_double_matrix(xdata,ndata,1);
	destroy_double_matrix(ydata,ndata,1);
	destroy_double_matrix(coeff,norder+1,1);
	destroy_double_matrix(pd,norder+1,1);

	return res;
}





int tde_timer_fun(int istep, int imissed)
{
	int ichan,imeas,dac_imeas,status;
//  int ittt;
	double time,temp_adc,temp_enc;
//  double val1,val2,temp_debug;

#ifndef USB
//	user command
	perform_user_command_delayed();
#endif
	
//	current time
	time=tde_get_time();

    /* Adjust dac rate
     * Write every tde_dac_ndown samples
     */
	dac_imeas=1;
	if ( tde_dac_idown<tde_dac_ndown ) {
		tde_dac_idown++;
		dac_imeas=0;
	}
	
	if ( dac_imeas==1 ) {
//        DEBUG        
//        printf("dac_imeas = 1\n");
		if ( tde_dac_idown==tde_dac_ndown ){
            /* Reset ignored samples counter due to downsampling of DAC */  
            tde_dac_idown=0;
        }
		
        /* MRonde: Is there a limit on number of DACs */
		for (ichan=0; ichan<tde_ndac; ichan++) {
			status=ec_dac_write_chan(fg_get_val(time,ichan), ichan);
            #ifdef LOG1
                /* Log channel 0 only */
                if(ichan == 0){
                    if(istep < MAXLOG1){
                        tde_log1_data[istep] = fg_get_val(time,ichan);
                        tde_log1_time[istep] = time;
                    }
                }
            #endif
            /* DEBUG FUNGEN
            temp_debug = fg_get_val(time,ichan);
            if (temp_debug != 0.0) {
                printf("%d \t %f \n",ichan,temp_debug);
            }
             */
		}
        /* MRonde (20110511) Make write values effective */
        ec_send();
	} /* End of DAC downsampling 1108 */

	if ( tde_go== -1 ) {
	    return 1;
	}

	if ( !tde_started ) {
		tde_reset_time();
		tde_started=1;
	}

    /* Adjust sampling frequency
     * Measure every tde_ndown samples
     */
	imeas=1;
	if ( tde_idown<tde_ndown ) {
        /* printf("Downsampling ADC\n"); */
		tde_idown++;
		imeas=0;
	}
	
	if ( imeas==1 ){
		if ( tde_idown==tde_ndown ){
            /* Reset ignored samples counter due to downsampling of ADC and ENC*/
            tde_idown=0;
        }
		
        
        /* MRonde (20110511) Refresh mapping before reading inputs */
        ec_receive();
        
        /* MRonde 2011-05-29: 
         * Changed to logical port api and split read of adc and enc,
         * since their number can be unequal
         */
        
        /* MRonde: Read adc's up to MAXCHAN and store in cyclic buffer */
        for ( ichan=0; ichan<tde_nadc || ichan < MAXCHAN; ichan++) {
            /* Read ADC */
            status=ec_adc_read_chan(&temp_adc, ichan);
            #ifdef LOG1
                /* Log channel 0 only */
                if(ichan == 0){
                    if(istep < MAXLOG1){
                        tde_log1_data2[istep] = temp_adc;
                    }
                }            
            #endif
            /* Check for clipping */
            status=ec_adc_get_clipped_port(&(tde_clip[ichan]), ichan);
            /* DEBUG
             * printf("eseng reports: port_id=%d \t clipped=%d\n",ichan,tde_clip[ichan]);
             */
            #ifdef DUMMY
                if( ichan == 0 ){
                    temp_adc = sin(2*M_PI*1*time);
                } else {
                    temp_adc = ichan+1;
                }
            #endif
            /*fix: 2015-02-03 repaired offset [RvdMolengraft]*/
            /* Store in cyclic buffer */
			(&tde_data[0]+tde_buffer_idx*MAXCHAN*2+ichan)[0]=temp_adc+tde_adc_offset[ichan];            
            
        } /* End of adc channel read */
        
        /* MRonde: Read enc's up to MAXCHAN and store in cyclic buffer */
        for ( ichan=0; ichan<tde_nenc || ichan < MAXCHAN; ichan++) {
            /* Read ENC */
			status=ec_enc_read_chan(&temp_enc, ichan);
            #ifdef DUMMY
                temp_enc = ichan+9;
            #endif
            /* Store in cyclic buffer */
            (&tde_data[0]+tde_buffer_idx*MAXCHAN*2+MAXCHAN+ichan)[0]=temp_enc-tde_enc0[ichan];        
            /* Correction of enc0 is not required anymore (also for EL* ?)*/
        }        
        
        /* tde_buffer_idx = [0 MAXSAMP-1] sample number within cyclic buffer
         * MAXCHAN adc and MAXCHAN enc inputs are supported
         * static double tde_data[MAXSAMP*MAXCHAN*2];
         *
         * adc0(0) - ADC value of adc 0 (logical index) at time k=0
         * [adc0-adc7 enc0-enc7](0)
         */
		
		if ( tde_buffer_idx == tde_data_idx ) {
			if ( tde_read==0 ) {
				tde_valid = 0;
				printf("tde_timer_fun: non-read data is overwritten.\n");	
			}
		}

//		check trigger condition
		if ( tde_trigger==1 ) {

//			current trigger signal value
			tde_trig_val_hist[0]=fir_filter((&tde_data[0]+tde_buffer_idx*MAXCHAN*2+tde_trig_signal)[0],MAXCHAN+1);
			if (tde_buffer_idx==0) {
//				there's no history yet...
				tde_trig_val_hist[1]=tde_trig_val_hist[0];
			}
		
//			no triggering before preroll has been passed
			if (tde_trig_state==TRIG_STATE_WAIT) {
//				if (tde_buffer_idx>=igetmax((int) (tde_trig_preroll*tde_nsamp),tde_ntaps))
//				PATCHED 01-11-05
				if (tde_buffer_idx>=igetmax((int) ((tde_trig_preroll+1)*tde_nsamp),tde_ntaps)) {
					tde_trig_state=TRIG_STATE_GO;
				}
			}

			if (tde_trig_state==TRIG_STATE_GO) {
				switch (tde_trig_type) {
				case 0:
//					positive slope
					if ((tde_trig_val_hist[0]-tde_trig_val_hist[1])>0.0) {
						if ((tde_trig_val_hist[0]-tde_trig_level)*(tde_trig_level-tde_trig_val_hist[1])>0.0) {
#ifdef DEBUG
							printf("Triggered: positive slope\n");
							printf("hist[0]-hist[1]=%f\n",tde_trig_val_hist[0]-tde_trig_val_hist[1]);
#endif
							tde_trig_state=TRIG_STATE_DONE;
							tde_triggered=1;
							tde_frame_idx=((int) (tde_trig_preroll*tde_nsamp))+tde_ntaps/2;
						}
					}
					break;
				case 1:
//					negative slope
					if ((tde_trig_val_hist[0]-tde_trig_val_hist[1])<0.0) {
						if ((tde_trig_val_hist[0]-tde_trig_level)*(tde_trig_level-tde_trig_val_hist[1])>0.0) {
#ifdef DEBUG
							printf("Triggered: negative slope\n");							
							printf("hist[0]-hist[1]=%f\n",tde_trig_val_hist[0]-tde_trig_val_hist[1]);
#endif
							tde_trig_state=TRIG_STATE_DONE;
							tde_triggered=1;
							tde_frame_idx=((int) (tde_trig_preroll*tde_nsamp))+tde_ntaps/2;
						}
					}
					break;
				}
			}

//			store trigger history
			tde_trig_val_hist[1]=tde_trig_val_hist[0];

		}

		if (tde_frame_idx>=tde_nsamp-1) {
#ifdef DEBUG
			printf("tde_frame_idx = %d \t time = %g\n",tde_frame_idx,rtc_get_time());
#endif
			if ( (tde_trigger==0) | (tde_triggered==1) ) {
//				store index and pointer	
				tde_data_idx=tde_buffer_idx+1-tde_nsamp;

//				cycle compensation
				if (tde_data_idx<0) {tde_data_idx=MAXSAMP+tde_data_idx;}

//				frame complete
				tde_valid=1;

//				not yet read
				tde_read=0;

//				next frame
				tde_frame_idx=-1;
			}

			if ( (tde_trigger==1) & (tde_triggered==1) ) {
//				reset trigger
				tde_triggered=0;
				tde_trig_state=TRIG_STATE_WAIT;
			}
		}
		
//		next frame sample
		tde_frame_idx++;

//		next buffer sample
		if (tde_buffer_idx==MAXSAMP-1) {
//			let's cycle...
			tde_buffer_idx=-1;
		}
		tde_buffer_idx++;
	
	} /**/

    	return 0;

}





int tde_timer_fun_simulink(double* pmeas, double* psteer)
{
	int ichan,imeas,dac_imeas;
//  int ioff,ilink,status,ittt;
	double time,ttt;
//  double val1,val2;

//	current time
	time=tde_get_time();

//	adjust dac rate
	dac_imeas=1;
	if (tde_dac_idown<tde_dac_ndown) {
		tde_dac_idown++;
		dac_imeas=0;
	}
	
	if (dac_imeas==1) {
		if (tde_dac_idown==tde_dac_ndown) tde_dac_idown=0;
		
		for (ichan=0;ichan<4;ichan++) {
			psteer[ichan]=fg_get_val(time,ichan);
		}
//		printf("psteer[0]=%f\n",psteer[0]);
	}

	if (tde_go==0) return 0;
	if (tde_go==-1) return 1;

	if (!tde_started) {
		tde_reset_time();
		tde_started=1;
	}

//	adjust sampling frequency
	imeas=1;
	if (tde_idown<tde_ndown) {
		tde_idown++;
		imeas=0;
	}
	
	if (imeas==1) {
		if (tde_idown==tde_ndown) tde_idown=0;
		
		for (ichan=0;ichan<4;ichan++)
		{
//		    read adc channels
		    ttt=pmeas[ichan]+tde_adc_offset[ichan];
//		    store in cyclic buffer
		    (&tde_data[0]+tde_buffer_idx*MAXCHAN*2+ichan)[0]=ttt;
										
//		    read enc channels (sf_qadscope features 4+4 channels)
		    ttt=pmeas[4+ichan];
//		    store in cyclic buffer
		    tde_last_enc[ichan]=ttt-tde_enc0[ichan];
		    (&tde_data[0]+tde_buffer_idx*MAXCHAN*2+MAXCHAN+ichan)[0]=tde_last_enc[ichan];
		}

		if (tde_buffer_idx==tde_data_idx) {
			if (tde_read==0) {
				tde_valid=0;
//				printf("tde_timer_fun: non-read data is overwritten.\n");	
			}
		}

//		check trigger condition
		if (tde_trigger==1) {

//			current trigger signal value
			tde_trig_val_hist[0]=fir_filter((&tde_data[0]+tde_buffer_idx*MAXCHAN*2+tde_trig_signal)[0],MAXCHAN+1);
			if (tde_buffer_idx==0) {
//				there's no history yet...
				tde_trig_val_hist[1]=tde_trig_val_hist[0];
			}
		
//			no triggering before preroll has been passed
			if (tde_trig_state==TRIG_STATE_WAIT) {
//				if (tde_buffer_idx>=igetmax((int) (tde_trig_preroll*tde_nsamp),tde_ntaps)) {
//				PATCHED 01-11-05
				if (tde_buffer_idx>=igetmax((int) ((tde_trig_preroll+1)*tde_nsamp),tde_ntaps)) {
					tde_trig_state=TRIG_STATE_GO;
				}
			}

			if (tde_trig_state==TRIG_STATE_GO) {
				switch (tde_trig_type) {
				case 0:
//					positive slope
					if ((tde_trig_val_hist[0]-tde_trig_val_hist[1])>0.0) {
						if ((tde_trig_val_hist[0]-tde_trig_level)*(tde_trig_level-tde_trig_val_hist[1])>0.0) {
#ifdef DEBUG
							printf("Triggered: positive slope\n");
							printf("hist[0]-hist[1]=%f\n",tde_trig_val_hist[0]-tde_trig_val_hist[1]);
#endif
							tde_trig_state=TRIG_STATE_DONE;
							tde_triggered=1;
							tde_frame_idx=((int) (tde_trig_preroll*tde_nsamp))+tde_ntaps/2;
						}
					}
					break;
				case 1:
//					negative slope
					if ((tde_trig_val_hist[0]-tde_trig_val_hist[1])<0.0) {
						if ((tde_trig_val_hist[0]-tde_trig_level)*(tde_trig_level-tde_trig_val_hist[1])>0.0) {
#ifdef DEBUG
							printf("Triggered: negative slope\n");							
							printf("hist[0]-hist[1]=%f\n",tde_trig_val_hist[0]-tde_trig_val_hist[1]);
#endif
							tde_trig_state=TRIG_STATE_DONE;
							tde_triggered=1;
							tde_frame_idx=((int) (tde_trig_preroll*tde_nsamp))+tde_ntaps/2;
						}
					}
					break;
				}
			}

//			store trigger history
			tde_trig_val_hist[1]=tde_trig_val_hist[0];

		}

		if (tde_frame_idx>=tde_nsamp-1) {
#ifdef DEBUG
			printf("tde_frame_idx = %d\n",tde_frame_idx);
#endif
			if ( (tde_trigger==0) | (tde_triggered==1) ) {
//				store index and pointer	
				tde_data_idx=tde_buffer_idx+1-tde_nsamp;

//				cycle compensation
				if (tde_data_idx<0) {tde_data_idx=MAXSAMP+tde_data_idx;}

//				frame complete
				tde_valid=1;

//				not yet read
				tde_read=0;

//				next frame
				tde_frame_idx=-1;
			}

			if ( (tde_trigger==1) & (tde_triggered==1) ) {
//				reset trigger
				tde_triggered=0;
				tde_trig_state=TRIG_STATE_WAIT;
			}
		}
		
//		next frame sample
		tde_frame_idx++;

//		next buffer sample
		if (tde_buffer_idx==MAXSAMP-1) {
//			let's cycle...
			tde_buffer_idx=-1;
		}
		tde_buffer_idx++;

	}

	return EC_SUCCESS;

}





int tde_check_data(void)
{
    /* Return 1 is data is valid */
	double time;
	
	if (tde_valid==1) {
		time=tde_get_time();
		return 0;
	} else {
		return 1;
	}
}





int tde_say_ready(void)
{
	tde_read=1;
	tde_valid=0;

	return 0;
}





int tde_get_data(double* pdata, int ldata)
{
    /* Get ldata samples from the buffer */
    int i,j,n;

	if (ldata<tde_nsamp*MAXCHAN*2) {
		n=ldata;
	} else {
		n=tde_nsamp*MAXCHAN*2;
	}

    /* Index within buffer */
	j=tde_data_idx*MAXCHAN*2;
	for (i=0;i<n;i++) {
		pdata[i]=tde_data[j];
        /* Check if end of buffer reached -> let's cycle...*/
		if (j==MAXSAMP*MAXCHAN*2-1) {
			j=-1;
		}
		j++;
	}

	return tde_valid;
}





int tde_get_info(double* pmaxchan, double* pmaxframe)
{
	pmaxchan[0]=(double) MAXCHAN;
	pmaxframe[0]=(double) MAXFRAME;
	
	return 0;
}





static int tde_get_link_id(int* pichan, int* pilink)
{
        int i,j;
    
//      reconstruct channel id and link id from channel number
        i=*pichan;
        j=0;
        while (i>1) {
                j++;
                i=i-2;
        }

//      channel id
        *pichan=i;    

//      link id
        *pilink=j;
    
        return 0;
}





int tde_trigger_on(int isignal, int itype, double level, double preroll, double relfreq, int nsamp)
{
	int i,go_bak;
	
//	halt measurement
	go_bak=tde_go;
	tde_go=0;
	fg_notify(0);
	rtc_usleep(10000);
	
//	discard old data
	tde_valid=0;
	
//	restart buffer/frame
	tde_buffer_idx=0;

//	trigger on, but not yet triggered
	tde_trigger=1;
	tde_triggered=0;
	tde_trig_state=TRIG_STATE_WAIT;

//	set trigger signal
	tde_trig_signal=isignal;

//	set trigger type
	tde_trig_type=itype;

//	set trigger level
	tde_trig_level=level;

//	set trigger preroll
	tde_trig_preroll=preroll;

//	initialize trigger fir filter
	tde_ntaps=fir_init(MAXCHAN+1);
#ifdef DEBUG
    printf("trigger on isignal=%d \t",isignal);
	printf("tde_ntaps = %d\n",tde_ntaps);
#endif

//	design trigger fir filter
	fir_design(relfreq,MAXCHAN+1);

//	restore tde_go
	tde_go=go_bak;
	i=(tde_go==1) & (tde_linked==1);
	fg_notify(i);
	 
	return 0;
}





int tde_trigger_off(void)
{
	tde_trigger=0;
	tde_triggered=0;

	return 0;
}





int tde_check_clipping(double* pclip)
{
	int i;
	
	for (i=0;i<MAXCHAN;i++) {
		pclip[i]=(double) tde_clip[i];
/*        printf("channel %d tde_clip = %d\n",i,tde_clip[i]);*/
	}
	return 0;
}





int tde_set_adc_state(int istat, int ichan)
{
	int status;

	status=0;

	if (tde_is_simulink) {
	    tde_adc_state[ichan]=istat;
	    return 0;
	}
    
    ec_adc_set_state(istat,ichan);

	return status;
}





int tde_get_adc_state(double* pstat, int port_id)
{
    /* Get the adc state, 0 is disabled, 1 is enabled */
	int istat=0;

	if (tde_is_simulink) {
	    pstat[0]=(double) tde_adc_state[port_id];
	    return 0;
	}

    ec_adc_get_state(&istat,port_id);
    
    printf("tde_get_adc_state reports: chan = %d state = %d\n",port_id,istat);
    
	pstat[0]=(double) istat;

	return istat;
}





int tde_set_enc_state(int istat, int ichan)
{
	int status;

	status=0;
	
	if (tde_is_simulink) {
	    tde_enc_state[ichan]=istat;
	    return 0;
	}

    ec_enc_set_state(istat,ichan);

	return status;
}





int tde_get_enc_state(double* pstat, int port_id)
{
    /* Get the enc state, 0 is disabled, 1 is enabled */
	int istat=0;

	if (tde_is_simulink) {
	    pstat[0]=(double) tde_enc_state[port_id];
	    return 0;
	}

    ec_enc_get_state(&istat,port_id);
    
    printf("tde_get_enc_state reports: chan = %d state = %d\n",port_id,istat);
    
	pstat[0]=(double) istat;

	return istat;
}





int tde_set_dac_state(int istat, int ichan)
{
	int status,i,ilink;

	status=0;
	
	if (tde_is_simulink) {
	    tde_dac_state[ichan]=istat;
	    return 0;
	}

	i=ichan;
	tde_get_link_id(&i,&ilink);
#ifndef USB
	status=1;
    /* replace td_dac_set_enable_chan(istat,i,ilink);*/
#endif

	return status;
}





int tde_get_dac_state(double* pstat, int ichan)
{
    /* Jbest: What should istat be initially? */
    int istat = 0;
    int status,i,ilink;
	
	if (tde_is_simulink) {
	    pstat[0]=(double) tde_dac_state[ichan];
	    return 0;
	}

	i=ichan;
	tde_get_link_id(&i,&ilink);
	status=1;
    /* replace td_dac_get_enable_chan(&istat,i,ilink);*/
	pstat[0]=(double) istat;

	return status;
}





int tde_set_adc_offset(double offset, int ichan)
{
	tde_adc_offset[ichan]=offset;
	
	return 0;
}





int tde_get_adc_offset(double* poffset, int ichan)
{
	poffset[0]=tde_adc_offset[ichan];
	
	return 0;
}





int tde_init_enc(int ichan)
{
	int i,ilink;
	double ttt;

	i=ichan;
	tde_get_link_id(&i,&ilink);

	if (tde_is_simulink) {
//	    current count value becomes reset value
	    tde_enc0[ichan]=tde_last_enc[ichan];	        
	} else {
//	    current count value becomes reset value
//	    printf("Encoder reset at chan %d of link %d\n",i,ilink);
	    ec_enc_read_chan(&ttt, i);
	    tde_enc0[ichan]=ttt;
	}

	return 0;
}






int tde_aafilter_on(double freq, int iconfig, int ilink)
{
    /* Jbest: What should status be initially? */
	int status=0;

	if (tde_is_simulink) {
	    return 0;
	}
#if 0
#ifdef USB
//	set aa-filter cut-off frequency
	status=td_adc_set_filter_freq_chan(freq,0,ilink,TD_CACHED);
	status=td_adc_set_filter_freq_chan(freq,1,ilink,TD_CACHED);

//	set aa-filter configuration
	status=td_adc_set_filter_enable_chan(iconfig,0,ilink,TD_CACHED);
	status=td_adc_set_filter_enable_chan(iconfig,1,ilink,TD_CACHED);
#else
//	set aa-filter cut-off frequency
	status=td_adc_set_filter_freq_chan(freq,0,ilink,TD_DIRECT);
	status=td_adc_set_filter_freq_chan(freq,1,ilink,TD_DIRECT);

//	set aa-filter configuration
	status=td_adc_set_filter_enable_chan(iconfig,0,ilink,TD_DIRECT);
	status=td_adc_set_filter_enable_chan(iconfig,1,ilink,TD_DIRECT);
#endif
#endif
	return status;
}





int tde_aafilter_off(int ilink)
{
    /* Jbest: What should status be initially? */
	int status=0;
	
	if (tde_is_simulink) {
	    return 0;
	}
#if 0
#ifdef USB
	status=td_adc_set_filter_enable_chan(AQI_ADC_INPUT_FILTER_DISABLED,0,ilink,TD_CACHED);
	status=td_adc_set_filter_enable_chan(AQI_ADC_INPUT_FILTER_DISABLED,1,ilink,TD_CACHED);	
#else
	status=td_adc_set_filter_enable_chan(AQI_ADC_INPUT_FILTER_DISABLED,0,ilink,TD_DIRECT);
	status=td_adc_set_filter_enable_chan(AQI_ADC_INPUT_FILTER_DISABLED,1,ilink,TD_DIRECT);	
#endif
#endif
	return status;
}





int tde_set_range(int irange, int ichan, int ilink)
{
    /* Jbest: What should status be initially? */
	int status=0;

	if (tde_is_simulink) {
	    return 0;
	}
#if 0
#ifdef USB
	status=td_adc_set_range_chan(irange,ichan,ilink,TD_CACHED);
#else
	status=td_adc_set_range_chan(irange,ichan,ilink,TD_DIRECT);
#endif
#endif
	return status;
}





int tde_get_range(int ichan, int ilink)
{
//	int status = 0;
    /* Jbest: What should irange be initially? */
	int irange = 0;
#if 0    
	td_dev_caps dc;
	if (tde_is_simulink) {
	    return 7;
	}

	td_get_dev_caps(&dc,ilink);
	if (dc.type==TD_QAD) {
		irange=7;
//		printf("tde_get_range: irange=%d\n",irange);

	} else {
		status=td_adc_get_range_chan(&irange,ichan,ilink);
	}
#endif
	return irange;
}





int tde_get_filter_enable(int ichan, int ilink)
{
//	int status;
    /* Jbest: What should ifilt be initially? */
	int ifilt = 0;

	if (tde_is_simulink) {
	    return 0;
	}
#if 0
	status=td_adc_get_filter_enable_chan(&ifilt,ichan,ilink);
#endif	
	return ifilt;
}






int tde_get_maxfreq(double* pmaxfreq)
{
	pmaxfreq[0]=tde_maxfreq;
	
	return 0;
}





int tde_get_obsiv(double* pobsiv)
{
	pobsiv[0]=((double) tde_nsamp)/(tde_maxfreq/(((double) tde_ndown)+1.0));

	return 0;
}





int tde_link2meas(int ilinked)
{
	int i;
	
	tde_linked=ilinked;
	i= (tde_go==1) & (tde_linked==1);
	fg_notify(i);
	
	return 0;
}





int tde_link_is_active(void)
{
	return (tde_go==1) & (tde_linked==1);
}





int tde_get_adc_enc_state(double* pallstat)
{
	int status,i;
	
	for (i=0;i<MAXCHAN;i++) {
	    status=tde_get_adc_state(pallstat+i,i);
	    status=tde_get_enc_state(pallstat+MAXCHAN+i,i);
	}

	return status;
}

