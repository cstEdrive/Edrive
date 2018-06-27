/*
 *  Test for encoder soft-reset functionality
 *
 *  M.J.C. Ronde, 2011
 *  M.J.G. v.d. Molengraft, 2010, test01.c
 *  Eindhoven University of Technology
 *
 *  Changelog:
 *  20110409    -   Initial version (adapted from test01.c)
 */
#include <stdio.h>
#include "../../Libs/ec/ec.h"

#define NSAMPLES 100
#define ICHAN 1
#define ILINK 0


/* link_id starts at 0 */

int main(void)
{
        int i,iret;
        double encvalues[NSAMPLES];
        double x1;     

        if ( (iret=ec_start("eth0"))!=EC_SUCCESS ) {
                printf("foutje %d.\n", iret);
                return -1;
        }
                
        for (i=0; i<NSAMPLES; i++) {
                /* printf("i = %d\n", i); */
                if(i == (NSAMPLES/2)){
                    printf("Reset\n");
                    iret = ec_Ebox_enc_reset_chan(ICHAN, ILINK);
                    printf("iret = %d \n",iret);
                }
                ec_Ebox_enc_read_chan(&x1, ICHAN, ILINK);
                encvalues[i] = x1;
                ec_io();
                usleep(50000);
        }

        ec_stop();
   
        for (i=0; i<NSAMPLES; i++) {
                printf("%d = %f \n", i,encvalues[i]);
        }
        
        return 0;
}

