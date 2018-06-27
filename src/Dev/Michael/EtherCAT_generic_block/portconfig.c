/*
 *  Test for write_ports_config_file
 *
 *  M.J.C. Ronde, 2011
 *  M.J.G. v.d. Molengraft, 2010, test01.c
 *  Eindhoven University of Technology
 *
 *  Changelog:
 *  20110415    -   Initial version (adapted from test01.c)
 */
#include <stdio.h>
#include "../../../Libs/ec/ec.h"

/* link_id and chan_id start at 0 */

int main(void)
{
        int iret, i, port_id;
        double x1, x2;
        FILE *fp;

        if ( (iret=ec_start("eth0"))!=EC_SUCCESS ) {
                printf("foutje %d.\n", iret);
                return -1;
        }

        fp = fopen("ports.h","w");        
        ec_write_ports_config_file(fp, 1);            
        fclose(fp);
        
        fp = fopen("ports.m","w");
        ec_write_ports_config_file(fp, 2);
        fclose(fp);
        
        ec_stop();

        return 0;
}

