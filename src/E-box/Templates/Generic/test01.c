
#include <stdio.h>
#include "../../Libs/ec/ec.h"

/* link_id and chan_id start at 0 */

int main(void)
{
        port_adc padc;
        port_dac pdac;
        int iret, i, port_id;
        double x1, x2;

        if ( (iret=ec_start("eth0"))!=EC_SUCCESS ) {
                printf("foutje %d.\n", iret);
                return -1;
        }

        port_id=0;
        ec_get_adc_port_capabilities(port_id, &padc);
        printf("adc%d: %d, %d, %d, %f, %f, %d, 0x%x, %s\n", port_id, padc.port_id, padc.link_id, padc.chan_id, padc.resolution, padc.range, padc.enabled, padc.dev_id, padc.dev_str);
        port_id++;
        ec_get_adc_port_capabilities(port_id, &padc);
        printf("adc%d: %d, %d, %d, %f, %f, %d, 0x%x, %s\n", port_id, padc.port_id, padc.link_id, padc.chan_id, padc.resolution, padc.range, padc.enabled, padc.dev_id, padc.dev_str);

        port_id=0;
        ec_get_dac_port_capabilities(port_id, &pdac);
        printf("dac%d: %d, %d, %d, %f, %f, %d, 0x%x, %s\n", port_id, pdac.port_id, pdac.link_id, pdac.chan_id, pdac.resolution, pdac.range, pdac.enabled, pdac.dev_id, pdac.dev_str);
        port_id++;
        ec_get_dac_port_capabilities(port_id, &pdac);
        printf("dac%d: %d, %d, %d, %f, %f, %d, 0x%x, %s\n", port_id, pdac.port_id, pdac.link_id, pdac.chan_id, pdac.resolution, pdac.range, pdac.enabled, pdac.dev_id, pdac.dev_str);

        for (i=0; i<1000; i++) {
                printf("i = %d\n", i);
                ec_Ebox_dac_write_chan(0.001, 0, 0);
                ec_Ebox_dac_write_chan(1.0, 1, 0);
                ec_Ebox_adc_read_chan(&x1, 0, 0);
                ec_Ebox_adc_read_chan(&x2, 1, 0);
                printf("adc0 =%f, adc1 = %f\n", x1, x2);
                if ( (iret=ec_io())!=EC_SUCCESS ) {
                        printf("I/O fout %d\n", iret);
                }
                usleep(50000);
        }

        ec_stop();

        return 0;
}

