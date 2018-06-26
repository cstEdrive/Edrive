#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "timer.h"

#define FREQ	12000
#define NSAMP	( (int) (1*60*FREQ+1) )
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


    fp=fopen("ttt","w");
    
	printf("NSAMP = %d\n",NSAMP);
	
//	  ptim=malloc(NSAMP*sizeof(double));
//	  if (ptim==NULL) {
//	    printf("Foutje!!!\n");
//	    return -1;
//	  }
  	
  	printf("test_timer - this test should take exactly %f minutes\n",((double) NSAMP-1)/(((double) FREQ)*60));
	rtc_timer(FREQ, the_apc);

    for (i=0;i<NSAMP;i++) {
	fprintf(fp,"%f\n",tim[i]);
    }
    
//    free(ptim);
    
    fclose(fp);
	
	return 0;
}
