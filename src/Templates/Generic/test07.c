/*   TEST07
 *   Test for encoder reset functionality on index pulse for E/BOX    
 *   
 *   Usage: Run from command line ./test07 with root privileges   
 *      - At NSAMPLES/2 index pulse triggering is enable
 *      - If trigger by the index pulse the RAW encoder value get reset to zero
 *      - To obtain zero encoder output count0 needs to be reset as well   
 *
 *   Inputs: NSAMPLES, ILINK, ICHAN can be changed by changing defines and recompling 
 *   
 *   Outputs: Outputs encoder values and status register to terminal
 *   
 *   See also 

 *   References: 
 *   
 *   Changes: 
 *   20111210 - Initial version (M.J.C. Ronde) 
 *   
 *   
 *   M.J.C. Ronde (2011-12-10) 
 *   ________________________________
 *   Eindhoven University of Technology
 *   Dept. of Mechanical Engineering
 *   Control Systems Technology group
 *   PO Box 513, WH -1.127
 *   5600 MB Eindhoven, NL
 *   T +31 (0)40 247 2798
 *   F +31 (0)40 246 1418
 *   E m.j.c.ronde@tue.nl
 */ 

#include <stdio.h> 
#include "../../Libs/ec/ec.h" 
 
#define NSAMPLES 100 
#define ICHAN 0 
#define ILINK 0 
 
 
/* link_id starts at 0 */ 
 
int main(void) 
{ 
        int i,iret,status_last; 
        double data[NSAMPLES][2]; 
        double encvalue,status;  
         
 
        if ( (iret=ec_start("eth0"))!=EC_SUCCESS ) { 
                printf("foutje %d.\n", iret); 
                return -1; 
        } 
                 
        for (i=0; i<NSAMPLES; i++) {                 
                if(i == 0){ 
                    /* Write 0 to control register*/ 
                    ec_Ebox_control_write_chan(0,ICHAN,ILINK);                     
                    /* status_last is read at first sample */                     
                    ec_Ebox_status_read_chan(&status, ICHAN, ILINK); 
                    status_last = (int) status; 
                } 
                if(i == (NSAMPLES/2)){ 
                    printf("Reset on index pulse i=%d\n",i); 
                    ec_Ebox_control_write_chan(1,ICHAN,ILINK); 
                } 
                 
                /* Read encoder and status */ 
                ec_Ebox_enc_read_chan(&encvalue, ICHAN, ILINK); 
                ec_Ebox_status_read_chan(&status, ICHAN, ILINK); 
                data[i][0] = encvalue; 
                data[i][1] = status;   
                 
                /* If status is different from status last and is 1, 
                 * then the encoder is reset by the index pulse 
                 * RAW value corresponds to index pulse (rising pulse) 
                 * To get the actual value to zero count0 should be reset 
                 * as well. 
                 */ 
                if((int) status != status_last && (int) status == 1){ 
                    printf("Index pulse is triggered\n"); 
                    /* Reset count0 */ 
                    ec_Ebox_enc_zero_chan(ICHAN,ILINK); 
                    /* Write 0 to control register again */ 
                    ec_Ebox_control_write_chan(0,ICHAN,ILINK);                     
                } 
                 
                status_last = (int) status; 
                ec_io(); 
                usleep(50000); 
        } 
 
        ec_stop(); 
    
        for (i=0; i<NSAMPLES; i++) { 
                printf("%d = %f \t status=%f\n", i,data[i][0],data[i][1]); 
        } 
         
        return 0; 
} 
 
