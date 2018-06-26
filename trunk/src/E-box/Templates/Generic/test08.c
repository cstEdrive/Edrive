/*
 *  Test for DAC and ADC values
 *
 *  M.J.C. Ronde, 2011
 *  M.J.G. v.d. Molengraft, 2010, test01.c
 *  Eindhoven University of Technology
 *
 *  Changelog:
 *  20110415    -   Initial version (adapted from test01.c)
 */
#include <stdio.h>
#include "../../Libs/ec/ec.h"

#define NSAMPLES 10000
#define ILINK 0


/* link_id starts at 0 */

int main(void)
{
        int i,iret;
        double adcvalues[NSAMPLES][2];
        double x0, x1;     
        FILE *fp;

        if ( (iret=ec_start("eth0"))!=EC_SUCCESS ) {
                printf("foutje %d.\n", iret);
                return -1;
        }
                
        for (i=0; i<=NSAMPLES; i++) {
                ec_Ebox_adc_read_chan(&x0, 0, ILINK);
                ec_Ebox_adc_read_chan(&x1, 1, ILINK);
                adcvalues[i][0] = x0;
                adcvalues[i][1] = x1;
                ec_Ebox_dac_write_chan(((double)i/NSAMPLES)*10 , 0, ILINK);
                ec_Ebox_dac_write_chan(((double)i/NSAMPLES)*-10 , 1, ILINK);
                ec_io();
                usleep(10000);
        }

        ec_stop();
        
        fp = fopen("test08.txt","w");
        for (i=0; i<NSAMPLES; i++) {
                fprintf(fp,"%d \t %f \t %f \t %f\n", i,((double)i/NSAMPLES)*10, adcvalues[i][0], adcvalues[i][1]);
        }
        fclose(fp);
        
        return 0;
}

