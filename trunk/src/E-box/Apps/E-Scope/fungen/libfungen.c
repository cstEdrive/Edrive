
// function generator

// Ren� van de Molengraft, November, 2001
// Revision history: January, 15th, 2003: added FIR-filtered band-limited noise signal
//										  (IIR elliptic was removed)
//					 December, 1st, 2003: generator got its own clock
//					 February, 6th, 2004: clock replaced by delta t increments
//										  for the purpose of exact simulation
//					 March, 14th, 2004: added link to measurement
//					 June, 30th, 2004: linux-port
//					 September, 3rd, 2004: changed tde_link_is_active into fg_notify to resolve linkage order
//								problem
//					 February, 9th, 2005: added chirp signal
// Michael Ronde, August, 2011
//					 August, 25th, 2011: Added header files for correct compilation (time.h and timer.h)
//                   August, 28th, 2011: Removed unused variables            

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if 0
#ifdef USB
#include <td_usb.h>
#else
#include <td.h>
#endif
#endif

#include "fungen.h"
#include <remez.h>
#include <fir.h>
#include "../engine/engine.h"
#include "../../../Libs/Timer_posix_AK/timer.h"


static int fg_type[MAXCHAN];
static int fg_power[MAXCHAN];
static int fg_onoff = 0;
static double fg_amp1[MAXCHAN];
static double fg_amp2[MAXCHAN];
static double fg_freq1[MAXCHAN];
static double fg_freq2[MAXCHAN];
static double fg_std[MAXCHAN];
static double fg_offset[MAXCHAN];
// static FILE *fp = NULL;
static double fg_maxfreq;
static double fg_obsiv;
static double fg_phase[MAXCHAN][MAXFRAME];
static int fg_k[MAXCHAN];
static int fg_is_simulink;

/* clock variables */
// static double fgc_t0 = 0.0;
// static double fgc_freq = 0.0;
static double fg_time;
// static double fg_arg = -1.0;
static int fg_ilinked = 0;
static double fg_t0 = 0.0;
static int fg_link_is_active;
static double fg_timt0;

// void _init()
// {
// //	initialize variables
// 	fg_onoff=0;
// 	fp=NULL;
// //	clock variables
// 	fgc_t0=0.0;
// 	fgc_freq=0.0;
// 
// 	fg_arg=-1.0;
// 	fg_ilinked=0;
// 	fg_t0=0.0;
// }





// void _fini()
// {
// }





static int nearest(double x)
{
    if (x>=0.0) {
	return (int) ((double) x+0.49999999999);
    } else {
	return (int) ((double) x-0.49999999999);
    }
}





double** create_double_matrix(int nrows, int ncols)
{
	int irow;
	double** a;
	
//	declare matrix dynamically

	a=(double**) calloc(nrows,sizeof(double*));

	for (irow=0;irow<nrows;irow++) {
		a[irow]=(double*) calloc(ncols,sizeof(double));
	}

	return a;
}





/*
#ifdef USB
static struct timeval tv0,tva;

int fgc_reset_time(void)
{
  gettimeofday(&tv0,NULL);
  
  return 0;
}

double fgc_get_time(void)
{
  double t,wrk;
  
  gettimeofday(&tva,NULL);
  
  t=(double) (tva.tv_sec-tv0.tv_sec)+((double) (tva.tv_usec-tv0.tv_usec))/1000000.0;
  
  return t;
}
#else
*/
int fgc_reset_time(void)
{
    fg_timt0=rtc_get_nominal_time();

    return 0;
}

double fgc_get_time(void)
{
    double t;

    t=rtc_get_nominal_time()-fg_timt0;

    return t;
}
/*#endif*/





double sqval(double x)
{
	if (x>=0) {
		return 1;
	} else {
		return -1;
	}
}





double fg_get_val(double time, int ichan)
{
	int iframe,k1,k2,k,n,nlines;
	double t,x,u,a,b,freq,tim,f1,f2,sum;

	fg_time=fgc_get_time();

//	link has just been turned on
	if ( (fg_link_is_active==1) & (fg_ilinked==0) ) {
//		printf("link has just been turned on (t=%f s).\n",fg_time);
		fg_ilinked=1;
		fg_t0=fg_time;
	}

//	link has just been turned off
	if ( (fg_link_is_active==0) & (fg_ilinked==1) ) {
//		printf("link has just been turned off (t=%f s).\n",fg_time);
		fg_ilinked=0;
		fg_t0=0.0;
	}

	t=fg_time;

	if ( (fg_onoff==0) & (fg_link_is_active==0) ) {
		return 0.0;
	}

//	if (ichan==0) {printf("type=%d, power=%d\n",fg_type[ichan],fg_power[ichan]);}
	switch (fg_type[ichan])
	{
	case SINE:
		if (fg_power[ichan]==1) {
			return fg_amp1[ichan]*sin(2*PI*fg_freq1[ichan]*(t-fg_t0))+fg_offset[ichan];
//			ttt=fg_amp1[ichan]*sin(2*PI*fg_freq1[ichan]*(t-fg_t0))+fg_offset[ichan];
//			if (ttt>0.0) {printf("ttt = %f\n",ttt);}
//			return ttt;
		} else {
			return 0.0;
		}
	case RAND:
		if (fg_power[ichan]==1) {
			u=fg_amp1[ichan]*2.0*( ((double) rand())/((double) RAND_MAX)-0.5);
			x=fir_filter(u,ichan)+fg_offset[ichan];
			return x;
		} else {
			return 0.0;
		}
	case SQUARE:
		if (fg_power[ichan]==1) {
			return fg_amp1[ichan]*sqval(sin(2*PI*fg_freq1[ichan]*(t-fg_t0)))+fg_offset[ichan];
		} else {
			return 0.0;
		}
	case TWOTONE:
		if (fg_power[ichan]==1) {
			return fg_amp1[ichan]*sin(2*PI*fg_freq1[ichan]*(t-fg_t0))+fg_amp2[ichan]*sin(2*PI*fg_freq2[ichan]*(t-fg_t0))+fg_offset[ichan];
		} else {
			return 0.0;
		}
	case CHIRP:
#define EPS 0.000001
		if (fg_power[ichan]==1) {
//		    actual time
		    tim=t-fg_t0;
    
//		    check frequency bounds
//		    lower frequency
		    f1=fg_freq1[ichan];
//	    	    upper frequency
		    f2=0.5*(f1+fg_freq2[ichan]);

		    n=nearest(2.0*f2*fg_obsiv);
	    	    if (n==0) {n=1;}
		    f2=((double) n)/(2.0*fg_obsiv);

//	   	    new bandwidth
		    fg_freq2[ichan]=2.0*(f2-0.5*f1);
//		    bandwidth should be non-negative	    
		    if (fg_freq2[ichan]<0.0) {fg_freq2[ichan]=0.0;}

//		    frame id
		    iframe=((int) ((t-fg_t0+EPS)/fg_obsiv))+1;
//		    adjust time
		    tim=tim-((double) (iframe-1))*fg_obsiv;
//		    linearly increasing frequency
		    f2=0.5*(fg_freq1[ichan]+fg_freq2[ichan]);
		    a=(f2-fg_freq1[ichan])/fg_obsiv;
		    b=fg_freq1[ichan];
		    freq=a*tim+b;
		    return fg_amp1[ichan]*sin(2*PI*freq*tim)+fg_offset[ichan];
		} else {
		    return 0.0;
		}
	case MULTISINE:
		if (fg_power[ichan]==1) {
//		    actual time
		    tim=t-fg_t0;

//		    check frequency bounds
		    k1=nearest(fg_freq1[ichan]*fg_obsiv);
		    if (k1==0) {k1=1;}
		    fg_freq1[ichan]=((double) k1)/fg_obsiv;    
		    k2=nearest(fg_freq2[ichan]*fg_obsiv);
		    if (k2==0) {k2=1;}
		    fg_freq2[ichan]=((double) k2)/fg_obsiv;    
//		    line bounds
		    nlines=k2-k1+1;
		    sum=0.0;
		    for (k=k1;k<k1+nlines;k++) {
			sum=sum+(fg_amp1[ichan]/sqrt((double) nlines))*sin(2*PI*k*tim/fg_obsiv+fg_phase[ichan][k]);
		    }
		    return sum+fg_offset[ichan];
		} else {
		    return 0.0;
		}
	case WHISTLE:
//		return 0.0;
		if (fg_power[ichan]==1) {
//		    actual time
		    tim=t-fg_t0;

//		    lowest spectral line
		    k1=nearest(fg_freq1[ichan]*fg_obsiv);
		    if (k1==0) {k1=1;}

//		    highest spectral line		    
		    k2=nearest(fg_freq2[ichan]*fg_obsiv);
		    if (k2==0) {k2=1;}
		    
//		    k2 should be greater or equal k1
		    if (k1>k2) {k2=k1;};

//		    frame id
		    iframe=((int) ((t-fg_t0+EPS)/fg_obsiv))+1;
//		    adjust time
		    tim=tim-((double) (iframe-1))*fg_obsiv;
		    while (iframe>0) {
			iframe=iframe-(k2-k1+1);
		    }
		    iframe=iframe+(k2-k1+1);
		    
//		    actual spectral line
		    fg_k[ichan]=k1+(iframe-1);

		    return fg_amp1[ichan]*sin(2*PI*fg_k[ichan]*tim/fg_obsiv)+fg_offset[ichan];
		} else {
		    return 0.0;
		}
	default:
		return 0.0;
	}
}





int fg_start(int ichan)
{
	if (fg_type[ichan]==RAND) {
		srand((unsigned) time(NULL));
	}
	
	fg_power[ichan]=1;

	return 0;
}





int fg_stop(int ichan)
{
	fg_power[ichan]=0;

	return 0;
}





int fg_status(int ichan)
{
	return fg_power[ichan];
}





int fg_set_defaults(void)
{
	int i,j;

	fg_onoff=0;
	
	for (i=0;i<MAXCHAN;i++) {
		fg_power[i]=0;
		fg_type[i]=SINE;
		fg_amp1[i]=1.0;
		fg_amp2[i]=1.0;
		fg_freq1[i]=1.0;
		fg_freq2[i]=1.0;
		fg_offset[i]=0.0;
		fg_std[i]=1.0;
		fir_init(i);
		fg_k[i]=1;
		for (j=0;j<MAXFRAME;j++) {
//			random phase between +-PI
			fg_phase[i][j]=PI*2.0*( ((double) rand())/((double) RAND_MAX)-0.5);
		}
	}
	
//	initialize clock
	fgc_reset_time();
	
	return 0;
}





int fg_set_amp1(double amp, int ichan)
{
	fg_amp1[ichan]=amp;

	return 0;
}





int fg_get_amp1(double* amp, int ichan)
{
	amp[0]=fg_amp1[ichan];

	return 0;
}





int fg_set_amp2(double amp, int ichan)
{
	fg_amp2[ichan]=amp;

	return 0;
}





int fg_get_amp2(double* amp, int ichan)
{
	amp[0]=fg_amp2[ichan];

	return 0;
}





int fg_set_freq1(double freq, int ichan)
{
	int ittt,n;

//	temporary power off
	ittt=fg_power[ichan];
	fg_power[ichan]=0;

	if ((fg_type[ichan]==MULTISINE) | (fg_type[ichan]==WHISTLE)) {
//	    adjust multisine frequency bound
	    n=nearest(freq*fg_obsiv);
	    if (n==0) {n=1;}
	    freq=((double) n)/fg_obsiv;    

	    if (fg_type[ichan]==MULTISINE) {
//	        maximum bandwidth is 256 spectral lines due to cpu effort
	        if ( (fg_freq2[ichan]-freq)>(255.0/fg_obsiv) ) {
		    freq=fg_freq2[ichan]-(255.0/fg_obsiv);
		    if (freq<=0.0) {freq=1.0/fg_obsiv;}
		}
	    }
	}

	fg_freq1[ichan]=freq;

	if (fg_type[ichan]==RAND) {
//		design new fir filter
		fir_design(freq/fg_maxfreq,ichan);
	}

//	restore power setting
	fg_power[ichan]=ittt;
	
	return 0;
}





int fg_get_freq1(double* freq, int ichan)
{
	freq[0]=fg_freq1[ichan];

	return 0;
}





int fg_set_freq2(double freq, int ichan)
{
	int ittt,n;
	double f1,f2;

//	temporary power off
	ittt=fg_power[ichan];
	fg_power[ichan]=0;
	
	if (fg_type[ichan]==CHIRP) {
//	    adjust chirp bandwidth
//	    lower frequency
	    f1=fg_freq1[ichan];
//	    upper frequency
	    f2=0.5*(f1+freq);

	    n=nearest(2.0*f2*fg_obsiv);
	    if (n==0) {n=1;}
	    f2=((double) n)/(2.0*fg_obsiv);

//	    new bandwidth
	    freq=2.0*(f2-0.5*f1);
//	    bandwidth should be non-negative	    
	    if (freq<0.0) {freq=0.0;}
	}

	if ((fg_type[ichan]==MULTISINE) | (fg_type[ichan]==WHISTLE)) {
//          adjust multisine frequency bound
	    n=nearest(freq*fg_obsiv);
	    if (n==0) {n=1;}
	    freq=((double) n)/fg_obsiv;

	    if (fg_type[ichan]==MULTISINE) {
//	        maximum bandwidth is 256 spectral lines due to cpu effort
		if ( (freq-fg_freq1[ichan])>(255.0/fg_obsiv) ) {
		    freq=fg_freq1[ichan]+(255.0/fg_obsiv);
		}
	    }
	    
	    if (fg_type[ichan]==WHISTLE) {
//		freq2 should be greater or equal freq1
		if (freq<fg_freq1[ichan]) {
		    freq=fg_freq1[ichan];
		}
	    }
	    
	}

	fg_freq2[ichan]=freq;

//	restore power setting
	fg_power[ichan]=ittt;

	return 0;
}





int fg_get_freq2(double* freq, int ichan)
{
	freq[0]=fg_freq2[ichan];

	return 0;
}





int fg_set_offset(double offset, int ichan)
{
	fg_offset[ichan]=offset;

	return 0;
}





int fg_get_offset(double* offset, int ichan)
{
	offset[0]=fg_offset[ichan];

	return 0;
}




int fg_set_type(int itype, int ichan)
{
	int ittt;
	
//	temporary power off
	ittt=fg_power[ichan];
	fg_power[ichan]=0;

	fg_type[ichan]=itype;

	if (fg_type[ichan]==RAND) {
//		design new fir filter
		fir_design(fg_freq1[ichan],ichan);
	}

//	restore power setting
	fg_power[ichan]=ittt;

	return 0;
}




int fg_get_type(double* type, int ichan)
{
	type[0]=(double) fg_type[ichan];

	return 0;
}





int fg_set_onoff(int ipower)
{
	if (ipower==1) {
//		time line restarts at zero
		fgc_reset_time();
		fg_time=0.0;
	}

	fg_onoff=ipower;
		
	return 0;
}




int fg_get_onoff(double* power)
{
	power[0]=(double) fg_onoff;

	return 0;
}





int fg_notify(int link_is_active)
{
	if (link_is_active==1) {
		fg_link_is_active=1;
	} else {
		fg_link_is_active=0;
	}
	
	return 0;
}





#if 0
#define MAXHOLDORDER	3

int fg_set_hold_order(int holdorder, int ichan)
{
	int ilink;

	if (fg_is_simulink) {
	    return 0;
	}

//	find link id (assumes ilink 0 or 1)
	if (ichan<2) {
	    ilink=0;
	} else {
	    ichan=ichan-2;
	    ilink=1;
	}
	
	if (holdorder>MAXHOLDORDER) {
//	    hardware hoh
	    holdorder=holdorder-MAXHOLDORDER;
//	    set hoh order
	    td_dac_set_filter_order_chan(holdorder,ichan,ilink,TD_CACHED);
//	    switch on hardware hoh
	    td_dac_set_filter_enable_chan(1,ichan,ilink,TD_CACHED);
	} else {
//	    switch off hardware hoh
	    td_dac_set_filter_enable_chan(0,ichan,ilink,TD_CACHED);
//	    set hold order
	    td_dac_set_filter_order_chan(holdorder,ichan,ilink,TD_CACHED);
	}
	
	return 0;
}





int fg_get_hold_order(double* pholdorder, int ichan)
{
	int ienable;
	int iorder;
	int ilink;
    
	if (fg_is_simulink) {
	    pholdorder[0]=0.0;
	    return 0;
	}

//	find link id (assumes ilink 0 or 1)
	if (ichan<2) {
	    ilink=0;
	} else {
	    ichan=ichan-2;
	    ilink=1;
	}

//	get hold order
        td_dac_get_filter_order_chan(&iorder,ichan,ilink);
//	check for hardware hoh
	td_dac_get_filter_enable_chan(&ienable,ichan,ilink);
	
	if (ienable==1) {
//		hardware hoh
	 	pholdorder[0]=(double) (iorder+MAXHOLDORDER);
	} else {
//		software hoh
	 	pholdorder[0]=(double) iorder;	
	}
	
	return 0;
}
#endif

/* MRonde (20110503) Empty functions to enable compiling */
int fg_set_hold_order(int holdorder, int ichan)
{
    return 0;
}

int fg_get_hold_order(double* pholdorder, int ichan)
{
    return 0;
}



int fg_put_maxfreq(double maxfreq)
{
    fg_maxfreq=maxfreq;

    return 0;
}





int fg_put_obsiv(double obsiv)
{
    fg_obsiv=obsiv;
    
    return 0;
}





int fg_put_issim(int is_simulink)
{
    fg_is_simulink=is_simulink;

    return 0;
}
