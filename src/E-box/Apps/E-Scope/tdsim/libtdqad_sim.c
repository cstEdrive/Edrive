
//#define QADSCOPE2
//#define DEBUG

#define EMULATION_SELECT		4

#define EMULATION_TRUCK			0
#define EMULATION_LOOPBACK		1
#define EMULATION_TWOMASSSPRING	2
#define EMULATION_MASSSPRING	3
#define EMULATION_TWOTONE		4

// TUeDACS/1 qad specific functions

// René van de Molengraft, December, 2003
// Revision history:	December, 17, start
//						January, 23th, 2004 simulation version
//						January, 26th, 2004 system implementation

// IMPORTANT: as long as a particular channel has not been enabled,
// read/write commands on that channel do not have any effect!
// A channel can be enabled in two ways:
// 1. by initializing it (init_chan)
// 2. by explicitly enabling it (enable_chan)

#include <string.h>
#include <math.h>
#include <stdio.h>
#include "tdIo.h" 
#include <td_usb.h>

// MAXFREQ is read from QADScope...
//#include "../../QadScope/v25/tdext/tdext.h"

// two adc channels per device
static tdport_adc qad_adc[2*TD_MAXLINKS];
// two dac channels per device
static tdport_dac qad_dac[2*TD_MAXLINKS];
// two encoder channels per device
static tdport_enc qad_enc[2*TD_MAXLINKS];
// one digital inport per device
static tdport_din qad_di[TD_MAXLINKS];
// one digital outport per device
static tdport_dout qad_do[TD_MAXLINKS];
// one simulation system
static tdsys the_sys;
static int adc_current_mode;
static int dac_current_mode;

void _init()
{
//	initialize variables
	adc_current_mode=0;
	dac_current_mode=0;
}





void _fini()
{
}





int qad_check_qadscope_version(void)
{	
	return 1;
}





static double absval(double x)
{
	if (x>=0) {
		return x;
	} else {
		return -x;
	}
}





static double signval(double x)
{
	if (x>=0) {
		return 1.0;
	} else {
		return -1.0;
	}
}





static double xsim[4]={0.0,0.0,0.0,0.0};
static double time=0.0;

#define M1	1350.0
#define M2	8650.0
#define K1	6.5e6
#define K2	4.4e5
#define B	4.31e4

#define STIFFNSS	15791.0
#define MASS		1.0
#define DAMP		0.5

#define STIFFNSS2	0.001
#define STIFFNESS	0.05
#define DAMPING		0.00001
#define MASS1		4.8e-6
#define MASS2		4.8e-6

#define PATOMASS	4.8e-6
#define PATOSTIFFNESS	0.22
#define PATODAMPING	0.0001
#define PATODAMPINGFW	0.0001

#define PI			3.14159265358979 

static int simulated_experiment(double* padc, double* penc, double *pdac)
{
#	define DT	0.000001
	int i,j,nsteps,emulation_select;
	double dxsim[4];
	double maxfreq,dt;

	emulation_select=EMULATION_SELECT;
//	maxfreq=tde_get_maxfreq(&maxfreq);
	maxfreq=4096.0;
		
	switch (emulation_select) {
	case EMULATION_TRUCK:
//		xsim[0]: wheel position
//		xsim[1]: wheel velocity
//		xsim[2]: chassis position
//		xsim[3]: chassis velocity

		dt=DT;
		nsteps=(int) (1.0/(maxfreq*dt));
		if (nsteps==0) {
		    nsteps=1;
		    dt=1.0/maxfreq;
		}
//		printf("nsteps=%d\n",nsteps);
		for (j=0;j<nsteps;j++) {
//			state equation
			dxsim[0]=xsim[1];
			dxsim[1]=(K1*(pdac[0]-xsim[0])+K2*(xsim[2]-xsim[0])+B*(xsim[3]-xsim[1]))/M1;
			dxsim[2]=xsim[3];
			dxsim[3]=(K2*(xsim[0]-xsim[2])+ B*(xsim[1]-xsim[3]))/M2;

			for (i=0;i<4;i++) {
				xsim[i]=xsim[i]+dt*dxsim[i];
			}
		}

//		adc0: road position
		padc[0]=pdac[0];
//		adc1: relative displacement between wheel and chassis
		padc[1]=xsim[0]-xsim[2];
//		adc3: wheel acceleration
		padc[2]=0.005*dxsim[1];
//		adc4: chassis acceleration
		padc[3]=0.1*dxsim[3];

		for (i=0;i<4;i++) {
			penc[i]=0.0;
		}
		break;
	case EMULATION_LOOPBACK:
//		adc: loopback dac
		for (i=0;i<4;i++) {
			padc[i]=pdac[i];
		}
		#ifdef DEBUG
		printf("loopback: dac0 = %f, adc0 = %f\n", pdac[0], padc[0]);
		#endif
		for (i=0;i<4;i++) {
			penc[i]=0.0;
		}
		break;
	case EMULATION_TWOMASSSPRING:
//		xsim[0]: motor position
//		xsim[1]: motor velocity
//		xsim[2]: load position
//		xsim[3]: load velocity

		dt=DT;
		nsteps=(int) (1.0/(maxfreq*dt));
		if (nsteps==0) {
		    nsteps=1;
		    dt=1.0/maxfreq;
		}

		for (j=0;j<nsteps;j++) {
//		    state equation
	    	    dxsim[0]=xsim[1];
		    dxsim[1]=(-PATODAMPINGFW*xsim[1]-PATOSTIFFNESS*(xsim[0]-xsim[2])-PATODAMPING*(xsim[1]-xsim[3])+pdac[0])/PATOMASS;
		    dxsim[2]=xsim[3];
		    dxsim[3]=(-PATOSTIFFNESS*(xsim[2]-xsim[0])-PATODAMPING*(xsim[3]-xsim[1]))/PATOMASS;

		    for (i=0;i<4;i++) {
			xsim[i]=xsim[i]+dt*dxsim[i];
		    }
		}

		for (i=0;i<4;i++) {
			padc[i]=0.0;
			penc[i]=0.0;
		}

//		adc0: input
		padc[0]=pdac[0];
//		enc0: motor position
		penc[0]=xsim[0];
//		enc1: load position
		penc[1]=xsim[2];
//		adc1: motor acceleration
		padc[1]=dxsim[1];

		break;	
	case EMULATION_MASSSPRING:
//		xsim[0]: position
//		xsim[1]: velocity

		dt=DT;
		nsteps=(int) (1.0/(maxfreq*dt));
		if (nsteps==0) {
		    nsteps=1;
		    dt=1.0/maxfreq;
		}

		for (j=0;j<nsteps;j++) {
//			state equation
			dxsim[0]=xsim[1];
			dxsim[1]=(-STIFFNSS*xsim[0]-DAMP*xsim[1]+pdac[0])/MASS;

			for (i=0;i<2;i++) {
				xsim[i]=xsim[i]+dt*dxsim[i];
			}
		}

//		adc0: input
		padc[0]=pdac[0];
//		adc1: position
		padc[1]=xsim[0];
//		adc3: velocity
		padc[2]=xsim[1];
//		adc4: acceleration
		padc[3]=dxsim[1];

		for (i=0;i<4;i++) {
			penc[i]=0.0;
		}
		break;
	case EMULATION_TWOTONE:
		padc[0]=sin(2*PI*325.0*time)+0.001*cos(2*PI*200.0*time);
		time=time+1.0/maxfreq;
		break;
	}
	return 1;
}





int qad_check_status(int status, char* pfname)
{
	return status;
}





int qad_adc_mode_select(int idx)
{
	return (qad_adc[idx].mode_select&QAD_MASK_MODE_SELECT);
}





int qad_dac_mode_select(int idx)
{
	return (qad_dac[idx].mode_select&QAD_MASK_MODE_SELECT);
}





int qad_enc_function_select(int idx)
{
//	return (qad_enc[idx].mode_select&QAD_MASK_ENC_FUNCTION_SELECT)>>4;
	return QAD_ENC_SINGLE_ENDED;
}





// sync adc channel settings with qad
int qad_adc_sync_chan(int ichan, int ilink)
{
	int status;

	status=TD_SUCCESS;

	return status;
}





// init and sync adc channel settings
int qad_adc_init_chan(int ichan, int ilink)
{
		int status;

		int ioff,idx,i;

		ioff=ilink*TD_OFFSET;
		idx=ichan+2*ilink;

	//	enable adc channel
		qad_adc[idx].enable=1;

	//	not yet clipped
		qad_adc[idx].clipped=0;

	//	default gains and offsets
		for (i=0;i<3*7;i++) {
			qad_adc[idx].offset[i]=0.0;
			qad_adc[idx].gain[i]=1.0;
		}

	//	sync with qad
		status=qad_adc_sync_chan(ichan,ilink);
		qad_check_status(status,"qad_adc_init_chan");

	//	reset adc channel
		qad_adc[idx].status=qad_adc[idx].status|QAD_MASK_RESET;

	//	set mode 0
		status=qad_adc_set_mode_chan(QAD_INDIVIDUAL_SOFTWARE_TRIGGER,ichan,ilink,TD_DIRECT);
		qad_check_status(status,"qad_adc_init_chan");

	return status;
}





// show adc settings
int qad_adc_show_chan(int ichan, int ilink)
{
	int status;

	int idx;

	idx=ichan+2*ilink;
	
	printf("qad_adc_channel_%d at link %d\n",ichan+1,ilink);
	printf("enabled =       %d\n",qad_adc[idx].enable);
	printf("voltage =	    %f\n",qad_adc[idx].voltage);
	printf("clipped =       %d\n",qad_adc[idx].clipped);

	printf("offset =		%f\n",qad_adc[idx].offset[0]);
	printf("gain =			%f\n",qad_adc[idx].gain[0]);

	printf("mode =          %d\n",qad_adc_mode_select(idx));
	printf(" \n");

	status=0;
	
	return status;
}





int qad_adc_sync_and_show_chan(int ichan, int ilink)
{
	int status;

	status=qad_adc_sync_chan(ichan,ilink);
	qad_check_status(status,"qad_adc_sync_and_show_chan");

	status=qad_adc_show_chan(ichan,ilink);
	qad_check_status(status,"qad_adc_sync_and_show_chan");

	return status;
}





// start ad conversion via software (mode 0)
int qad_adc_start_chan(int ichan, int ilink, int imode)
{
	int status;

	status=TD_SUCCESS;
	
	return status;
}






// read adc data
int qad_adc_read_chan(double *pdata, int ichan, int ilink, int imode)
{
	int status;

	int ioff,idx;
	UWORD res=0;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;
	
//	if ( (idx==0) & ( (imode==TD_CACHED) | (imode==TD_DIRECT) ) )	{
//		perform a simulation step
//		simulated_experiment(the_sys.adc_chan,the_sys.enc_chan,the_sys.dac_chan);
//	}
	
	if (!qad_adc[idx].enable) {
		pdata[0]=qad_adc[idx].voltage;
		return TD_DISABLED;
	}
		
	switch (imode) {
	case TD_DIRECT:
		#ifdef DEBUG
		printf("adc_read: adc[%d] = %f\n", idx, the_sys.adc_chan[idx]);
		#endif
		qad_adc[idx].voltage=the_sys.adc_chan[idx];
		qad_adc_postprocess_chan(res,idx);
		break;
	case TD_CACHED:
		qad_adc[idx].voltage=the_sys.adc_chan[idx];
//		qad_adc_postprocess_chan(res,idx);
		break;
	case TD_STORED:
		status=TD_SUCCESS;
		break;
	default:
		status=TD_ERROR;
		break;
	}

	pdata[0]=qad_adc[idx].voltage;

	return status;
}





int qad_adc_postprocess_chan(UWORD res, int idx)
{
	int status;

	if (!qad_adc[idx].enable) {
		return TD_DISABLED;
	}
	
//	compute corresponding voltage
	qad_adc[idx].voltage=qad_adc[idx].voltage-qad_adc[idx].offset[0];

//	check clipping condition
	qad_adc[idx].clipped=0;
	if (absval(qad_adc[idx].voltage)>QAD_ADC_MAXLEVEL) {
		qad_adc[idx].clipped=1;
		qad_adc[idx].voltage=signval(qad_adc[idx].voltage)*QAD_ADC_MAXLEVEL;
	}

	status=TD_SUCCESS;
	
	return status;
}





int qad_adc_set_mode_chan(int imode_chan, int ichan, int ilink, int imode)
{
	int status;
	
	int ioff,idx;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;

//	adjust imode_chan map for mode 1 and 2 (to 3 and 4)
	if (imode_chan>0) {
		imode_chan=imode_chan+2;
	}

//	store new mode
	qad_adc[idx].mode_select=(qad_adc[idx].mode_select&(~QAD_MASK_MODE_SELECT))|imode_chan;

	status=TD_SUCCESS;
	
	return status;
}





int qad_adc_get_mode_chan(int *pimode_chan, int ichan, int ilink)
{
	int status;

	int idx;
	
	idx=ichan+2*ilink;
	
	pimode_chan[0]=qad_adc_mode_select(idx);

	status=TD_SUCCESS;
	
	return status;
}





int qad_adc_set_enable_chan(int ienable, int ichan, int ilink)
{
	int status;

	int idx;

	idx=ichan+2*ilink;

	qad_adc[idx].enable=ienable;

	status=TD_SUCCESS;

	return status;
}





int qad_adc_get_enable_chan(int *pienable, int ichan, int ilink)
{
	int status;

	int idx;

	idx=ichan+2*ilink;

	pienable[0]=qad_adc[idx].enable;

	status=TD_SUCCESS;

	return status;
}





int qad_adc_get_clipped_chan(int *piclipped, int ichan, int ilink)
{
	int status;

	int idx;

	idx=ichan+2*ilink;

	piclipped[0]=qad_adc[idx].clipped;

	status=TD_SUCCESS;

	return status;
}





// sync dac channel settings with qad
int qad_dac_sync_chan(int ichan, int ilink)
{
	int status;

	status=TD_SUCCESS;

	return status;
}





// init and sync dac channel settings
int qad_dac_init_chan(int ichan, int ilink)
{
	int status;

	int ioff,idx;
	double eps=1.0;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;
	
//	enable adc channel
	qad_dac[idx].enable=1;

//	not yet clipped
	qad_dac[idx].clipped=0;

//	default gains and offsets
	qad_dac[idx].offset=0.0;
	qad_dac[idx].gain=1.0;

//	default zoh
	qad_dac[idx].order_select=0;
	
//	sync with qad
	status=qad_dac_sync_chan(ichan,ilink);
	qad_check_status(status,"qad_dac_init_chan");

//  reset dac channel
	qad_dac[idx].status=qad_dac[idx].status|QAD_MASK_RESET;

//	set mode 0
	status=qad_dac_set_mode_chan(QAD_INDIVIDUAL_SOFTWARE_TRIGGER,ichan,ilink,TD_DIRECT);
	qad_check_status(status,"qad_dac_init_chan");

//	reset dac to zero
	status=qad_dac_write_chan(0.0,ichan,ilink,TD_DIRECT);
	qad_check_status(status,"qad_dac_init_chan");

	return status;
}





// show dac settings
int qad_dac_show_chan(int ichan, int ilink)
{
	int status;

	int idx;
	
	idx=ichan+2*ilink;
	
	printf("qad_dac_channel_%d at link %d\n",ichan+1,ilink);
	printf("enabled =       %d\n",qad_dac[idx].enable);
	printf("voltage =		%f\n",qad_dac[idx].voltage);
	printf("offset =		%f\n",qad_dac[idx].offset);
	printf("gain =			%f\n",qad_dac[idx].gain);
	printf("mode =          %d\n",qad_dac_mode_select(idx));
	printf(" \n");

	status=TD_SUCCESS;
	
	return status;
}





int qad_dac_sync_and_show_chan(int ichan, int ilink)
{
	int status;

	status=qad_dac_sync_chan(ichan,ilink);
	qad_check_status(status,"qad_dac_sync_and_show_chan");

	status=qad_dac_show_chan(ichan,ilink);
	qad_check_status(status,"qad_dac_sync_and_show_chan");

	return status;
}





int qad_dac_write_chan(double data, int ichan, int ilink, int imode)
{
	int status;

	int ioff,idx;
	UWORD res;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;
	
	if (!qad_dac[idx].enable) {
		status=TD_DISABLED;
		data=0.0;
	}
	
	qad_dac[idx].voltage=data;
	status=qad_dac_preprocess_chan(&res,idx);
	
	the_sys.dac_chan[idx]=qad_dac[idx].voltage;

	if ( (idx==0) & ( (imode==TD_CACHED) | (imode==TD_DIRECT) ) )	{
//		perform a simulation step
		simulated_experiment(&(the_sys.adc_chan[0]), &(the_sys.enc_chan[0]), &(the_sys.dac_chan[0]));
		#ifdef DEBUG
		printf("uit se: adc0 = %f\n", the_sys.adc_chan[0]);
		#endif
	}

	return status;
}





int qad_dac_preprocess_chan(UWORD *pres, int idx)
{
	int status;

	qad_dac[idx].voltage=qad_dac[idx].voltage-qad_dac[idx].offset;
	
	if (qad_dac[idx].voltage>2.5) {qad_dac[idx].voltage=2.5;}
	if (qad_dac[idx].voltage<-2.5) {qad_dac[idx].voltage=-2.5;}

	status=TD_SUCCESS;
	
	return status;
}





int qad_dac_set_mode_chan(int imode_chan, int ichan, int ilink, int imode)
{
	int status;
	
	int ioff,idx;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;

//	adjust imode_chan map for mode 1 and 2 (to 3 and 4)
	if (imode_chan>0) {
		imode_chan=imode_chan+2;
	}

//	printf("qad_dac_set_mode_chan: imode_chan = %d\n",imode_chan);

//	store new mode
	qad_dac[idx].mode_select=(qad_dac[idx].mode_select&(~QAD_MASK_MODE_SELECT))|imode_chan;

	status=TD_SUCCESS;
	
	return status;
}





int qad_dac_get_mode_chan(int *pimode_chan, int ichan, int ilink)
{
	int status;

	int idx;
	
	idx=ichan+2*ilink;

	pimode_chan[0]=qad_dac_mode_select(idx);
	
	status=TD_SUCCESS;
	
	return status;
}





int qad_dac_set_enable_chan(int ienable, int ichan, int ilink)
{
	int status;

	int idx;

	idx=ichan+2*ilink;

	qad_dac[idx].enable=ienable;

	status=TD_SUCCESS;

	return status;
}





int qad_dac_get_enable_chan(int *pienable, int ichan, int ilink)
{
	int status;

	int idx;

	idx=ichan+2*ilink;

	pienable[0]=qad_dac[idx].enable;

	status=TD_SUCCESS;

	return status;
}





int qad_dac_set_filter_order_chan(int iorder, int ichan, int ilink, int imode)
{
	int status;

	status=TD_SUCCESS;

	return status;
}





int qad_dac_get_filter_order_chan(int *piorder, int ichan, int ilink)
{
	int status;

	int idx;
	
	idx=ichan+2*ilink;

	piorder[0]=qad_dac[idx].order_select;
	
	status=TD_SUCCESS;
	
	return status;
}





// sync enc channel settings with qad
int qad_enc_sync_chan(int ichan, int ilink)
{
	int status;

	status=TD_SUCCESS;

	return status;
}





// init and sync enc channel settings
int qad_enc_init_chan(int ichan, int ilink)
{
	int status;

	int ioff,idx;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;

//	enable enc channel
	qad_enc[idx].enable=1;

//	sync with qad
	status=qad_enc_sync_chan(ichan,ilink);
	qad_check_status(status,"qad_enc_init_chan");

//  reset enc channel
	qad_enc[idx].status=qad_enc[idx].status|QAD_MASK_RESET;

//	set single-ended quadrature count
//	status=qad_enc_set_function_chan(QAD_ENC_SINGLE_ENDED,ichan,ilink,TD_DIRECT);
//	qad_check_status(status,"qad_enc_init_chan");

//	clear encoders
	qad_enc[idx].status=qad_enc[idx].status|QAD_MASK_ENC_CLEAR_COUNTER_CHANNEL_0|QAD_MASK_ENC_CLEAR_COUNTER_CHANNEL_1;

	return status;
}





// show enc settings
int qad_enc_show_chan(int ichan, int ilink)
{
	int status;

	int idx;
	
	idx=ichan+2*ilink;
	
	printf("qad_enc_channel_%d at link %d\n",ichan+1,ilink);
	printf("enabled =       %d\n",qad_enc[idx].enable);
	printf("count =			%d\n",qad_enc[idx].count);
	printf("function =		%d\n",qad_enc_function_select(idx));
	printf(" \n");

	status=TD_SUCCESS;
	
	return status;
}





int qad_enc_sync_and_show_chan(int ichan, int ilink)
{
	int status;

	status=qad_enc_sync_chan(ichan,ilink);
	qad_check_status(status,"qad_enc_sync_and_show_chan");

	status=qad_enc_show_chan(ichan,ilink);
	qad_check_status(status,"qad_enc_sync_and_show_chan");

	return status;
}





// read enc data
int qad_enc_read_chan(double *pdata, int ichan, int ilink, int imode)
{
	int status;

	int ioff,idx;
	ULONG lres=0;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;
	
	if (!qad_enc[idx].enable) {
		pdata[0]=(double) qad_enc[idx].count;
		return TD_DISABLED;
	}
	
	switch (imode) {
	case TD_DIRECT:
		qad_enc[idx].count=(long) the_sys.enc_chan[idx];
		status=qad_enc_postprocess_chan(lres,idx);
		break;
	case TD_CACHED:
		qad_enc[idx].count=(long) the_sys.enc_chan[idx];
		status=qad_enc_postprocess_chan(lres,idx);
		break;
	case TD_STORED:
		status=TD_SUCCESS;
		break;
	default:
		status=TD_ERROR;
		break;
	}

	pdata[0]=(double) qad_enc[idx].count;

	return status;
}





// read home position
int qad_enc_ref(int *pidata, int ilink, int imode)
{
	int status;

	int ioff,idx,ichan=0;
	UWORD res;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;

	if ( (!qad_enc[idx].enable) & (!qad_enc[idx+1].enable) ) {
		pidata[0]=(int) qad_enc[idx].ref_detect;
		pidata[1]=(int) qad_enc[idx+1].ref_detect;
		return TD_DISABLED;
	}
	
	switch (imode) {
	case TD_DIRECT:
//		status=pbeGetWord(QAD_QUADRATURE_STATUS_WORD_REGISTER+ioff,&res,ichan,ilink,TD_DIRECT,TD_NO_POST);
		res=0;
		status=qad_enc_ref_postprocess_chan((ULONG) res,idx);
		break;
	case TD_CACHED:
		res=0;
//		status=pbeGetWord(QAD_QUADRATURE_STATUS_WORD_REGISTER+ioff,&res,ichan,ilink,TD_CACHED,TD_ENC_REF_POST);
		break;
	case TD_STORED:
		status=TD_SUCCESS;
		break;
	default:
		status=TD_ERROR;
		break;
	}

	pidata[0]=(int) qad_enc[idx].ref_detect;
	pidata[1]=(int) qad_enc[idx+1].ref_detect;

	return status;
}





int qad_enc_postprocess_chan(ULONG lres, int idx)
{
	int status;

	if (!qad_enc[idx].enable) {
		return TD_DISABLED;
	}
	
	status=TD_SUCCESS;
	
	return status;
}





int qad_enc_ref_postprocess_chan(ULONG lres, int idx)
{
	int status;

	status=TD_SUCCESS;
	
	return status;
}





int qad_enc_set_function_chan(int ifunc_chan, int ichan, int ilink, int imode)
{
	int status;
	
	int ioff,idx;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;

//	store new mode
//	qad_enc[idx].function_select=(qad_enc[idx].function_select&(~QAD_MASK_ENC_FUNCTION_SELECT))|ifunc_chan;

//	write to hardware
//	status=pbePutWord(enc_mode_select_reg[ichan]+ioff,qad_enc[idx].function_select,ilink,imode);

	status=0;
	
	return status;
}






int qad_enc_get_function_chan(int *pifunc_chan, int ichan, int ilink)
{
	int status;

	int idx;
	
	idx=ichan+2*ilink;

	pifunc_chan[0]=qad_enc_function_select(idx);
	
	status=TD_SUCCESS;
	
	return status;
}





int qad_enc_clear_chan(int ichan, int ilink, int imode)
{
	int status;

	int ioff,idx;

	ioff=ilink*TD_OFFSET;
	idx=ichan+2*ilink;
	
//	store new status
	qad_enc[idx].status=qad_enc[idx].status|QAD_MASK_ENC_CLEAR_COUNTER_CHANNEL_0|QAD_MASK_ENC_CLEAR_COUNTER_CHANNEL_1;

	status=TD_SUCCESS;
	
	return status;
}





int qad_enc_set_enable_chan(int ienable, int ichan, int ilink)
{
	int status;

	int idx;

	idx=ichan+2*ilink;

	qad_enc[idx].enable=ienable;

	status=TD_SUCCESS;

	return status;
}





int qad_enc_get_enable_chan(int *pienable, int ichan, int ilink)
{
	int status;

	int idx;

	idx=ichan+2*ilink;

	pienable[0]=qad_enc[idx].enable;

	status=TD_SUCCESS;

	return status;
}





// sync do port settings with qad
int qad_do_sync(int ilink)
{
	int status;

	status=TD_SUCCESS;

	return status;
}





// init and sync do port settings
int qad_do_init(int ilink)
{
	int status;

	int ioff,ibit;

	ioff=ilink*TD_OFFSET;
	
//	enable do port
	qad_do[ilink].enable=1;

//	sync with qad
	status=qad_do_sync(ilink);
	qad_check_status(status,"qad_do_init");

//  reset do channel
	qad_do[ilink].status=qad_do[ilink].status|QAD_MASK_RESET;

//	reset do to zero
	for (ibit=0;ibit<8;ibit++) {
		status=qad_do_write_bit(0.0,ibit,ilink,TD_DIRECT);
		qad_check_status(status,"qad_do_init");
	}

	return status;
}





// show do port settings
int qad_do_show(int ilink)
{
	int status;

	printf("qad_do_port at link %d\n",ilink);
	printf("enabled =       %d\n",qad_do[ilink].enable);
	printf("data =			%x\n",qad_do[ilink].data);
	printf(" \n");

	status=TD_SUCCESS;
	
	return status;
}





int qad_do_sync_and_show(int ilink)
{
	int status;

	status=qad_do_sync(ilink);
	qad_check_status(status,"qad_do_sync_and_show");

	status=qad_do_show(ilink);
	qad_check_status(status,"qad_do_sync_and_show");

	return status;
}





int qad_do_write_bit(double data, int ibit, int ilink, int imode)
{
	int status;

	int ioff,istate,imask;

	ioff=ilink*TD_OFFSET;

	if (!qad_do[ilink].enable) {
		status=TD_DISABLED;
		istate=1;
	}
	
	istate=1-((int) data);
	imask=1<<ibit;
	qad_do[ilink].data=(qad_do[ilink].data&(~imask))|(istate<<ibit);

//	TODO: what to do with data?

	return status;
}





int qad_do_set_enable(int ienable, int ilink)
{
	int status;

	qad_do[ilink].enable=ienable;

	status=TD_SUCCESS;

	return status;
}





int qad_do_get_enable(int *pienable, int ilink)
{
	int status;

	pienable[0]=qad_do[ilink].enable;

	status=TD_SUCCESS;

	return status;
}





// sync do port settings with qad
int qad_di_sync(int ilink)
{
	int status;

	status=TD_SUCCESS;

	return status;
}





// init and sync di port settings
int qad_di_init(int ilink)
{
	int status;

	int ioff,ibit;

	ioff=ilink*TD_OFFSET;
	
//	enable do port
	qad_di[ilink].enable=1;

//	sync with qad
	status=qad_di_sync(ilink);
	qad_check_status(status,"qad_di_init");

//  reset di channel
	qad_di[ilink].status=qad_di[ilink].status|QAD_MASK_RESET;

	for (ibit=0;ibit<8;ibit++) {
		status=qad_di_select_bit(ibit,ilink,TD_DIRECT);
	}

	return status;
}





// show do port settings
int qad_di_show(int ilink)
{
	int status;

	printf("qad_di_port at link %d\n",ilink);
	printf("enabled =       %d\n",qad_di[ilink].enable);
	printf("data =			%x\n",qad_di[ilink].data);
	printf("direction =		%x\n",qad_di[ilink].direction);
	printf(" \n");

	status=TD_SUCCESS;
	
	return status;
}





int qad_di_sync_and_show(int ilink)
{
	int status;

	status=qad_di_sync(ilink);
	qad_check_status(status,"qad_di_sync_and_show");

	status=qad_di_show(ilink);
	qad_check_status(status,"qad_di_sync_and_show");

	return status;
}





// read di data
int qad_di_read_bit(double *pdata, int ibit, int ilink, int imode)
{
	int status;

	int ioff,idx,ichan=0,imask;
	UWORD res;

	ioff=ilink*TD_OFFSET;
	idx=ilink;

	if (!qad_di[ilink].enable) {
		pdata[0]=(double) (1-((qad_di[ilink].data&imask)>>ibit));
		return TD_DISABLED;
	}
		
	switch (imode) {
	case TD_DIRECT:
//		TODO: di
		res=0;
//		status=pbeGetWord(QAD_INPORT_DATA_REGISTER+ioff,&res,ichan,ilink,TD_DIRECT,TD_NO_POST);
		status=qad_di_postprocess(res,ilink);
		break;
	case TD_CACHED:
		res=0;
//		status=pbeGetWord(QAD_INPORT_DATA_REGISTER+ioff,&res,ichan,ilink,TD_CACHED,TD_DI_POST);
		break;
	case TD_STORED:
		status=TD_SUCCESS;
		break;
	default:
		status=TD_ERROR;
		break;
	}

	imask=1<<ibit;
	pdata[0]=(double) (1-((qad_di[ilink].data&imask)>>ibit));

	return status;
}





int qad_di_set_enable(int ienable, int ilink)
{
	int status;

	qad_di[ilink].enable=ienable;

	status=TD_SUCCESS;

	return status;
}





int qad_di_get_enable(int *pienable, int ilink)
{
	int status;

	pienable[0]=qad_di[ilink].enable;

	status=TD_SUCCESS;

	return status;
}





int qad_do_select_bit(int ibit, int ilink, int imode)
{
	int status;

	int ioff,imask;

	ioff=ilink*TD_OFFSET;

	imask=1<<ibit;

	qad_di[ilink].direction=qad_di[ilink].direction|imask;

	status=TD_SUCCESS;
	
	return status;
}





int qad_di_select_bit(int ibit, int ilink, int imode)
{
	int status;

	int ioff,imask;

	ioff=ilink*TD_OFFSET;

	imask=1<<ibit;

	qad_di[ilink].direction=qad_di[ilink].direction&(~imask);

	status=TD_SUCCESS;
	
	return status;
}





int qad_di_postprocess(UWORD res, int idx)
{
	int status;

	int ilink;

	ilink=idx/2;
	
	if (!qad_di[ilink].enable) {
		return TD_DISABLED;
	}
	
//	sync data
	qad_di[idx].data=res;

	status=TD_SUCCESS;
	
	return status;
}





int qad_postprocess_chan(pbe_return_data retdat)
{
	int status;

	int i;
	
	for (i=0;i<retdat.ndata;i++) {
		switch (retdat.ipost[i]) {
		case TD_NO_POST:
			status=TD_SUCCESS;
			break;
		case TD_ADC_POST:
			status=qad_adc_postprocess_chan((UWORD) retdat.data[i],retdat.idx[i]);
			break;
		case TD_ENC_POST:
			status=qad_enc_postprocess_chan(retdat.data[i],retdat.idx[i]);
			break;
		case TD_ENC_REF_POST:
			status=qad_enc_ref_postprocess_chan(retdat.data[i],retdat.idx[i]);
			break;
		case TD_DI_POST:
			status=qad_di_postprocess((UWORD) retdat.data[i],retdat.idx[i]);
			break;
		default:
			status=TD_ERROR;
			break;
		}
	}

	return status;
}





int qad_is_simulator(void)
{
	int status,emulation_select;
	char msgtxt[50];

//	if (qad_check_qadscope_version()!=1) {
//		MessageBox(NULL,"Wrong version of QADScope detected. Install QADScope v2.0 LE first!","QadScope Warning",MB_ICONEXCLAMATION|MB_OK|MB_TOPMOST);
//		return 0;
//	}
	emulation_select=EMULATION_SELECT;
	
	switch (emulation_select) {
	case EMULATION_TRUCK:
		sprintf(msgtxt,"Simulated experiment (Truck)\n");
		break;
	case EMULATION_LOOPBACK:
		sprintf(msgtxt,"Simulated experiment (Loopback)\n");
		break;
	case EMULATION_TWOMASSSPRING:
		sprintf(msgtxt,"Simulated experiment (Two-mass-spring)\n");
		break;	
	case EMULATION_MASSSPRING:
		sprintf(msgtxt,"Simulated experiment (MassSpring)\n");
		break;
	case EMULATION_TWOTONE:
		sprintf(msgtxt,"Simulated experiment (5 juli 2004)\n");
		break;
	}
	
//	MessageBox(NULL,msgtxt,"QadScope Warning",MB_ICONEXCLAMATION|MB_OK|MB_TOPMOST);

	status=1;

	return status;
}
