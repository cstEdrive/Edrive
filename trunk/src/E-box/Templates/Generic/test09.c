
#include <stdio.h>
#include <time.h>
#include "../../Libs/ec/ec.h"
#include "../../Libs/Timer_posix_AK/timer.h"


#define NSAMP   16000
#define FREQ    16000.0

#define NSEC_PER_SEC_F 1000000000.0

static FILE *fp;
static double stats[NSAMP][2];
static struct timespec tv0,tva;

int user_apc(int,int);


int main(void)
{
        int iret, i;
        double x1, x2;
        
        printf("Test should take approx %g seconds \n",(double) (NSAMP/FREQ));

        if ( (iret=ec_start("eth0"))!=EC_SUCCESS ) {
                printf("foutje %d.\n", iret);
                return -1;
        }

        clock_gettime(CLOCK_REALTIME, &tv0);
        rtc_timer(FREQ, user_apc, 75);

        ec_stop();
        
        fp = fopen("sampletimes","w");
        for(i = 0; i < NSAMP; i++){
            fprintf(fp,"%g\n",stats[i][1]);
        }
        fclose(fp);

        return 0;
}

int user_apc(int icnt, int imissed)
{
    double t;
    
    clock_gettime(CLOCK_REALTIME, &tva);
    t=(double) (tva.tv_sec-tv0.tv_sec)+((double) (tva.tv_nsec-tv0.tv_nsec))/NSEC_PER_SEC_F;
    
    stats[icnt][0] = rtc_get_time();
    stats[icnt][1] = t;
    
    ec_receive();
    ec_send();
    
    if(icnt < NSAMP){
        return 0;
    } else {
        return -1;
    }
}

