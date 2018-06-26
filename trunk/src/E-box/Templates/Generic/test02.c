/*
 *  Unit test for digital i/o
 *  Loopback test
 *
 *  M.J.C. Ronde, 2011
 *  Eindhoven University of Technology
 *
 *  Changelog:
 *  20110228    -   Changed order of I/O cycle and RW 
 *              -   Verified DI and DO
 *  20110126    -   Added possibility to send and recieve from different link_id 
 *              -   Changed timeout to prevent 100 ms timeout between packets
 *  20110125    -   Initial version
 */
#include <stdio.h>
#include "../../Libs/ec/ec.h"

/* link_id starts at 1 */
#define LINK_ID_RECIEVE     0
#define LINK_ID_SEND        0

/* TODO:
 * 
 */

int main(void)
{
        int iret, i, j, ichan;
        unsigned short int bitmask[8];
        double x[8];

        if ( (iret=ec_start("eth0"))!=EC_SUCCESS ) {
                printf("foutje %d.\n", iret);
                return -1;
        }

        for (i=0; i<255; i++) {
                printf("i = %d\n", i);              
                
                /* bit masks voor digital I/O */
                bitmask[0]=0x01;
                bitmask[1]=0x02;
                bitmask[2]=0x04;
                bitmask[3]=0x08;
                bitmask[4]=0x10;
                bitmask[5]=0x20;
                bitmask[6]=0x40;
                bitmask[7]=0x80;
                
                /* Convert interger to binary (single bits) using bitmask
                 *
                 *  0000 0100       i = 4
                 *  0000 0100 &     bitmask[2]
                 *  ------------
                 *  0000 0100
                 *
                 *  shift j times to make 1 or 0
                 */

                
                /* Write all outputs and do I/O */
                for(ichan=0; ichan<8; ichan++){
                    ec_Ebox_do_write_chan(((i & bitmask[ichan]) >> ichan), ichan, LINK_ID_SEND);
                }          
            
                ec_io();
                usleep(50000);         

                
                /* Do I/O and read all inputs */
                ec_io();
                usleep(50000);
                
                for(ichan=0; ichan<8; ichan++){              
                    ec_Ebox_di_read_chan(&x[ichan], ichan, LINK_ID_RECIEVE);
                }          
                
                /* Print send and read values */
                for(j=7; j>=0; j--){
                    printf("%d", ((i & bitmask[j]) >> j) );
                }
                printf(" \t ");
                for(j=7; j>=0; j--){                
                    printf("%1d", (int) x[j]);
                }
                printf("\n");                
        }

        ec_stop();

        return 0;
}

