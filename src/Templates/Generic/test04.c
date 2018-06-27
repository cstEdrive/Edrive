/*
 *  Test for CAN over Ethernet (CoE)
 *  for changing E-box parameters
 *
 *  M.J.C. Ronde, 2011
 *  M.J.G. v.d. Molengraft, 2010, test01.c
 *  Eindhoven University of Technology
 *
 *  Changelog:
 *  20110210    -   Initial version (adapted from test01.c)
 *  20111210    -   Added test for display
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../Libs/soem/ethercattype.h"
#include "../../Libs/ec/ec.h"

//#include "../../Libs/soem/ethercatcoe.h"


/* link_id starts at 0 */

/* TODO: filtered values are not correct, see line 372 from ec.c */
int main(void)
{
        int i,iret;
        double adc1, adc2,enc,ts, tsadc;
        par_ebox parameters;

        if ( (iret=ec_start("eth0"))!=EC_SUCCESS ) {
                printf("foutje %d.\n", iret);
                return -1;
        }

        printf("Parameters 0 or from last experiment \n");
        ec_Ebox_get_parameters(0, &parameters);
        printf("link_id = %d \t slave_id = %d , filter_ain = [%d,%d] \n",parameters.link_id,parameters.slave_id,parameters.filter_ain[0],parameters.filter_ain[1]);
        printf("hw_ver = %s \t sw_ver = %s \n",parameters.hw_ver,parameters.sw_ver);
        
        printf("Parameters 0 \n");
        parameters.filter_ain[0] = 0;
        parameters.filter_ain[1] = 0;
        ec_Ebox_set_parameters(0, &parameters);

        ec_Ebox_get_parameters(0, &parameters);
        printf("link_id = %d \t slave_id = %d , filter_ain = [%d,%d] \n",parameters.link_id,parameters.slave_id,parameters.filter_ain[0],parameters.filter_ain[1]);

        parameters.filter_ain[0] = 200;
        parameters.filter_ain[1] = 25;
        parameters.encoder_mode  = 1;
        ec_Ebox_set_parameters(0, &parameters);

        ec_Ebox_get_parameters(0, &parameters);
        printf("link_id = %d \t slave_id = %d , filter_ain = [%d,%d] \n",parameters.link_id,parameters.slave_id,parameters.filter_ain[0],parameters.filter_ain[1]);
        printf("encoder_mode = %d\n",parameters.encoder_mode);
        
        /* Write test to display */
        strncpy(parameters.textline1,"Test125",16);
        strncpy(parameters.textline2,"Test356",16);
        strncpy(parameters.textline3,"Test589",16);
        ec_Ebox_set_parameters(0, &parameters);
        
       
        
        for (i=0; i<1000; i++) {
                printf("i = %d\n", i);
                ec_Ebox_dac_write_chan(1.0, 0, 0);
                ec_Ebox_adc_read_chan(&adc1, 0, 0);
                ec_Ebox_adc_read_chan(&adc2, 1, 0);
                ec_Ebox_enc_read_chan(&enc, 0, 0);
                ec_Ebox_enc_read_chan(&ts, 1, 0);
                ec_Ebox_adc_read_timestamp(&tsadc, 0);
                printf("adc0 =%f, adc1 = %f, enc0=%f timestamp=%f %f\n", adc1, adc2, enc, ts, tsadc);
                ec_io();
                usleep(50000);
        }

        ec_stop();

        return 0;
}

