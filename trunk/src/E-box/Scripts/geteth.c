/*geteth.c*/ 

#include <stdlib.h>
#include <stdio.h>
#include "../Libs/ec/ec.h"

/* link_id and chan_id start at 0 */

int main(void)
{
        int iret;
       

        if ( (iret=ec_start("eth2"))==EC_SUCCESS ) {
            ec_stop();
            printf("\n\nEhtercat stack connected to eth2 ...\n");
            /*printf("\n\nEtherCAT stack connected to eth2, run changeeth(2) in matlab\n");*/
            return 2;
        }

        if ( (iret=ec_start("eth1"))==EC_SUCCESS ) {
            ec_stop();
            printf("\n\nEhtercat stack connected to eth1 ...\n");
            /*printf("\n\nEtherCAT stack connected to eth1, run changeeth(1) in matlab\n");*/
            return 1;
        }
        
        if ( (iret=ec_start("eth0"))==EC_SUCCESS ) {
            ec_stop();
            printf("\n\nEhtercat stack connected to eth0 ...\n");
            /*printf("\n\nEtherCAT stack connected to eth0, run changeeth(0) in matlab\n");*/
	    return 0;
        }
	
 
        printf("\n\nNo EtherCAT stack found, connect EtherCAT to pc and run as root\n");
        return -1;
        
}
