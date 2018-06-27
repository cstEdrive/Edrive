#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "timer.h"

#define FREQ	1000
#define NSAMP	( (int) (FREQ*20+1) )
#define PI  3.141592653
static double tprev=-1.0/FREQ;
static FILE* fp;
static double tim[NSAMP];

int the_apc(int icnt,int imissed)
{
    double t, u;
    
//    printf("NSAMP = %d\n", NSAMP);
   
    	if (icnt<NSAMP) {
	    //printf("icnt=%d\n",icnt);
	    tim[icnt]=rtc_get_time();
//            u = sin(2*PI*tim[icnt]);
//            ec_dac_write_chan(u, 0);
//	    fprintf(fp,"%f\n",t);

		return 0;
	} else {
		return 1;
	}
}





int main(void)
{
	int i;
	int hist[201];
	int hdt;

    fp=fopen("ttt","w");
    
	printf("NSAMP = %d\n\r",NSAMP);
	
//	  ptim=malloc(NSAMP*sizeof(double));
//	  if (ptim==NULL) {
//	    printf("Foutje!!!\n");
//	    return -1;
//	  }
  	
  	printf("test_timer - this test should take exactly %f minutes\n\r",((double) NSAMP-1)/(((double) FREQ)*60));
	rtc_timer(FREQ, the_apc, 1);

	printf("timer finished.\n\r");

//    for (i=0;i<NSAMP;i++) {
//	fprintf(fp,"%f\n",tim[i]);
//    }
    for (i=0;i<201;i++) { hist[i]=0; }	
    for (i=1;i<NSAMP;i++) {
	hdt=(tim[i]-tim[i-1])*1000000;
        if(hdt<0) { hdt=0; }
	if(hdt>200) { hdt=200; }
	hist[hdt]++;
    }
    for (i=0;i<200;i++) {
	fprintf(fp,"%d %d\n",i,hist[i]);
    }
    
//    free(ptim);
    
    fclose(fp);
	
	return 0;
}
