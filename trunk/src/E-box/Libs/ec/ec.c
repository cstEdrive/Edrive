// #define DEBUG
/** 
 \file ec.c
 \brief API layer for Simple Open EtherCAT master
 ec.c: API layer for Simple Open EtherCAT master
 
        History:
        M.J.G. van de Molengraft, 2007: initial version
        IME Technologies, 2008:         adapted for SOEM
        M.J.C. Ronde, 2010:             E-Box functionality added
        M.J.G. van de Molengraft, 30 january 2011: added device capability structures
                                  31 january 2011: added logical port API for adc, dac and enc
        M.J.C. Ronde,             16 april 2011: added logical port API for din and dout
                                  17 april 2011: added logical port API for pwm
                                  21 may 2011: added parameter structure for ebox (mapping from link_id to slave_id and adc summation filters )
		M.J.C. Ronde & R. v.d. Bogaert 12 juli 2011 : Added support for Beckhof EL3104   
        M.J.C. Ronde              05 september 2011: Added device string to port capability structure
        					 	  09 november 2011 : Bugfix for EL2004 en EL2008
                                  10 december 2011: Added hw/sw version to parameter structure, also added function for zeroing of count0 of E/BOX
                                  13 december 2011: Read latch value of EL5101 and reset count0 to zero functionality
 
 */

/*
        TODO

        port API for din, dout and pwm
*/

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
#include </usr/src/soem/ethercattype.h>
#include </usr/src/soem/ethercatbase.h>
#include </usr/src/soem/ethercatmain.h>
#include </usr/src/soem/ethercatcoe.h>
#include </usr/src/soem/ethercatconfig.h>
#include </usr/src/soem/nicdrv.h>
#include "ec.h"

/* Tag for private funtions*/
#define PRIVATE static

/* prototypes of private funtions */
PRIVATE int ec_set_adc_port_capabilities(int port_id, int link_id, int chan_id, double res, double range, int enabled, int dev_id, char *dev_str);
PRIVATE int ec_set_dac_port_capabilities(int port_id, int link_id, int chan_id, double res, double range, int enabled, int dev_id, char *dev_str);
PRIVATE int ec_set_enc_port_capabilities(int port_id, int link_id, int chan_id, int enabled, int dev_id, char *dev_str);
PRIVATE int ec_set_din_port_capabilities(int port_id, int link_id, int chan_id, int enabled, int dev_id, char *dev_str);
PRIVATE int ec_set_dout_port_capabilities(int port_id, int link_id, int chan_id, int enabled, int dev_id, char *dev_str);
PRIVATE int ec_set_pwm_port_capabilities(int port_id, int link_id, int chan_id, int enabled, int dev_id, char *dev_str);
PRIVATE int ec_setop(void);
PRIVATE int ec_Ebox_initialize_parameters(int link_id, int slave_id);
PRIVATE int ec_count_ports(void);

int rval;
uint16 ecvar;
struct timeval tv1,tv2,tv3;
struct sched_param schedp;
static char IOmap[4096];/* EtherCAT I/O buffer */
//static struct netif *ni;
static int bitmask[8];

/* EL5101 encoder module variable */
static int count0[MAX_NUMBER_OF_SLAVES], ifirst[MAX_NUMBER_OF_SLAVES], ilatch[MAX_NUMBER_OF_SLAVES], irevol[MAX_NUMBER_OF_SLAVES], prev_count[MAX_NUMBER_OF_SLAVES];

/* Ebox variables for encoder values (2 encoder per Ebox)*/
static int Ebox_count0[2*MAX_NUMBER_OF_SLAVES], Ebox_ifirst[2*MAX_NUMBER_OF_SLAVES], Ebox_irevol[2*MAX_NUMBER_OF_SLAVES], Ebox_prev_count[2*MAX_NUMBER_OF_SLAVES];

/* Encoder variables struct */
typedef struct tag_encoder_variables
{
    int count0;         /**< Encoder count at initilization */
    int ifirst;         /**< Variable to remember if the encoder needs to be resetted */ 
    int irevol;         /**< Number of revolutions */
    int prev_count;     /**< Encoder count at previous sample to detect under- and overflow*/
} encoder_variables;

/* number of devices of specific type */

static int nEK1100=0, nEL1008=0, nEL1018=0, nEL1014=0, nEL2004=0, nEL2008=0, nEL3102=0, nEL3104=0, nEL4132=0, nEL4038=0, nEL5101=0, nEbox=0;

/* number of ports of specific type */
typedef struct tag_portcount
{
	int adc;    /**< Counter for adc ports */
	int dac;    /**< Counter for dac ports */
	int enc;    /**< Counter for enc ports */
	int din;    /**< Counter for din ports */
	int dout;   /**< Counter for dout ports */
	int pwm;    /**< Counter for pwm ports */
} portcount;

static portcount n = {0,0,0,0,0,0};

/* number of parameter structures */
static int npar = 0;

/* pointer to array of pointers to port_adc */
static pport_adc ppadc;
static pport_dac ppdac;
static pport_enc ppenc;
static pport_din ppdin;
static pport_dout ppdout;
static pport_pwm pppwm;
/* pointer to array of pointers to par_ebox */
static ppar_ebox ppebox;

#include "structdata.dat"





static double absval(double x)
{
	if (x>=0) {
		return x;
	} else {
		return -x;
	}
}





static double signval(double x)
{
	if (x>=0) {
		return 1.0;
	} else {
		return -1.0;
	}
}




//! Read digital inputs from EL1008 module
/**
 * @param[out] pvalue Read bit from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL1008_di_read_chan(double *pvalue, int ichan, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH)  || (ilink>nEL1008-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>7) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue=(in_EL1008_[ilink]->inbits & bitmask[ichan] )>>ichan;

        *pvalue=(double) ivalue;

        return EC_SUCCESS;
}




//! Read digital inputs from EL1014 module
/**
 * @param[out] pvalue Read bit from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL1014_di_read_chan(double *pvalue, int ichan, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEL1014-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>7) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue=(in_EL1014_[ilink]->inbits & bitmask[ichan] )>>ichan;
         
        *pvalue=(double) ivalue;

        return EC_SUCCESS;
}




//! Read digital inputs from EL1018 module
/**
 * @param[out] pvalue Read bit from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL1018_di_read_chan(double *pvalue, int ichan, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEL1018-1) ) { return EC_ERR_INVALID_LINK_ID; }
  
        if ( (ichan<0) || (ichan>7) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue=(in_EL1018_[ilink]->inbits & bitmask[ichan] )>>ichan;

        *pvalue=(double) ivalue;

        return EC_SUCCESS;
}




//! Set digital outputs from EL2004 module
/**
 * @param[out] value Set bit from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL2004_do_write_chan(double value, int ichan, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEL2004-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>3) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue=(int) value;

        switch (ivalue) {
        case 0:
            out_EL2004_[ilink]->outbits = (out_EL2004_[ilink]->outbits & (~bitmask[ichan]));
            break;
        case 1:
            out_EL2004_[ilink]->outbits = (out_EL2004_[ilink]->outbits | bitmask[ichan]);
            break;
        }

        return EC_SUCCESS;
}




//! Set digital outputs from EL2008 module
/**
 * @param[out] value Set bit from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL2008_do_write_chan(double value, int ichan, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEL2008-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>7) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue=(int) value;

        switch (ivalue) {
        case 0:
            out_EL2008_[ilink]->outbits = (out_EL2008_[ilink]->outbits & (~bitmask[ichan]));
            break;
        case 1:
            out_EL2008_[ilink]->outbits = (out_EL2008_[ilink]->outbits | bitmask[ichan]);
            break;
        }

        return EC_SUCCESS;
}




//! Read adc from EL3102 module
/**
 * @param[out] pvalue Read adc value from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL3102_adc_read_chan(double *pvalue, int ichan, int ilink)
{
        int ivalue=0;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEL3102-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }

        switch (ichan) {
        case 0:
                ivalue=in_EL3102_[ilink]->invalue1;
                break;
        case 1:
                ivalue=in_EL3102_[ilink]->invalue2;
                break;
        }
        
        if (ivalue>32768) {
          ivalue=-(65535-ivalue);
        }

        *pvalue=((double) ivalue)*EL3102_RESOLUTION;

        return EC_SUCCESS;
}

//! Read adc from EL3104 module
/**
 * @param[out] pvalue Read adc value from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL3104_adc_read_chan(double *pvalue, int ichan, int ilink)
{
        int ivalue=0;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEL3104-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>3) ) { return EC_ERR_INVALID_CHAN_ID; }

        switch (ichan) {
        case 0:
                ivalue=in_EL3104_[ilink]->invalue1;
                break;
        case 1:
                ivalue=in_EL3104_[ilink]->invalue2;
                break;
        case 2: 
                ivalue=in_EL3104_[ilink]->invalue3;
                break;
        case 3: 
                ivalue=in_EL3104_[ilink]->invalue4;
                break;                
        }
        
        if (ivalue>32768) {
          ivalue=-(65535-ivalue);
        }

        *pvalue=((double) ivalue)*EL3104_RESOLUTION;

        return EC_SUCCESS;
}


//! Set dac value from EL4038 module
/**
 * @param[in] voltage_f Set dac value from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL4038_dac_write_chan(double voltage_f, int ichan, int ilink)
{
        double temp;
        int16 voltage_i;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEL4038-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>7) ) { return EC_ERR_INVALID_CHAN_ID; }

        temp=(voltage_f * 32767.0) / 10.0;
        voltage_i = (int16) temp;

        /* analog output */
        switch ( ichan) {
        case 0:
                out_EL4038_[ilink]->outvalue1=voltage_i;
                break;
        case 1:
                out_EL4038_[ilink]->outvalue2=voltage_i;
                break;
        case 2:
                out_EL4038_[ilink]->outvalue3=voltage_i;
                break;
        case 3:
                out_EL4038_[ilink]->outvalue4=voltage_i;
                break;
        case 4:
                out_EL4038_[ilink]->outvalue5=voltage_i;
                break;
        case 5:
                out_EL4038_[ilink]->outvalue6=voltage_i;
                break;
        case 6:
                out_EL4038_[ilink]->outvalue7=voltage_i;
                break;
        case 7:
                out_EL4038_[ilink]->outvalue8=voltage_i;
                break;
        }
	
        return EC_SUCCESS;
}




//! Set dac value from EL4132 module
/**
 * @param[in] voltage_f Set dac value from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL4132_dac_write_chan(double voltage_f, int ichan, int ilink)
{
        double temp;
        int16 voltage_i;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEL4132-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }

        temp=(voltage_f * 32767.0) / 10.0;
        voltage_i = (int16) temp;
        
        /* analog output */
        switch (ichan) {
        case 0:
                out_EL4132_[ilink]->outvalue1=voltage_i;
                break;
        case 1:
                out_EL4132_[ilink]->outvalue2=voltage_i;
                break;
        }

        return EC_SUCCESS;
}




//! Read encoder value from EL5101 module
/**
 * @param[out] pvalue Read encoder value from module at ichan/ilink
 * @param[in] ichan Channel on module
 * @param[in] ilink Linkid in module
 * @return Status
 */
int ec_EL5101_enc_read_chan(double *pvalue, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH)|| (ilink>nEL5101-1) ) { return EC_ERR_INVALID_LINK_ID; }

        ivalue=in_EL5101_[ilink]->invalue;

        if (ivalue>32768) {
            ivalue=-(65536-ivalue);
        }

        if (ifirst[ilink]==1) {
            count0[ilink]=ivalue;
            ifirst[ilink]=0;
            /*printf("zero counter at first sample\n");*/
        }

        /* correct with initial counter value */
        ivalue=ivalue-count0[ilink];

        /* keep track of over/underflow */
        if ( (ivalue-prev_count[ilink])>20000 ) { /* assume underflow! */
          irevol[ilink]--;
        } else if ( (prev_count[ilink]-ivalue)>20000 ) { /* assume overflow! */
          irevol[ilink]++;
        }

        /* store counter value */
        prev_count[ilink]=ivalue;

        ivalue=ivalue+irevol[ilink]*65536;

        *pvalue=(double) ivalue;

        return EC_SUCCESS;
}

int ec_EL5101_latch_read_chan(double *pvalue, int ilink)
{
        /* Reads the RAW encoder value on rising edge of index pulse*/
        int latch;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH)|| (ilink>nEL5101-1) ) { return EC_ERR_INVALID_LINK_ID; }
        
        if(ilatch[ilink] == 1){
            /* Set control bit to 0 to disable latch at first sample*/
            out_EL5101_[ilink]->control = 0;            
            ilatch[ilink] = 0;
        }
        
        /* Set control bit to 1 to enable latch */
        out_EL5101_[ilink]->control = 1;
        
        latch=in_EL5101_[ilink]->latch;
#ifdef DEBUG        
        printf("latchvalue = %d\n",latch);
#endif        
        *pvalue=(double) latch;

        return EC_SUCCESS;
}    

int ec_EL5101_enc_zero_chan(int ilink)
{           
        /* Set count0 value to zero */         
        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEL5101-1) ) { return EC_ERR_INVALID_LINK_ID; }             
        
        /* Set initial count to zero */
        count0[ilink] = 0;
        
        return EC_SUCCESS;
}
        
        





/*================================================================================
 *  Ebox functionality (Inputs)
 *================================================================================
 */
int ec_Ebox_adc_read_chan(double *pvalue, int ichan, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue = in_Ebox_[ilink]->ain[ichan];
        
        /* Filter values 0 or 1 correspond both to taking one sample, for higher filter values divide by n*/
        if((ppebox+ilink)->filter_ain[ichan] < (uint8) 2) {
            *pvalue = ((double) ivalue)*EBOX_ADC_RESOLUTION; /* +-10 V with 18 bit ADC, 100 microVolt per bit */
        } else {
            *pvalue = (1/((double) (ppebox+ilink)->filter_ain[ichan]))*((double) ivalue)*EBOX_ADC_RESOLUTION; /* +-10 V with 18 bit ADC, 100 microVolt per bit */
        }
        
        return EC_SUCCESS;
}

int ec_Ebox_adc_read_timestamp(double *pvalue, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        ivalue = (int) in_Ebox_[ilink]->tsain;

#ifdef DEBUG        
        printf("%d\n",in_Ebox_[ilink]->tsain);
#endif
        *pvalue = ((double) ivalue);
        
        return EC_SUCCESS;
}




int ec_Ebox_enc_read_chan(double *pvalue, int ichan, int ilink)
{       
        /* Read encoder channels 
         * ichan 0 = encoder 1
         * ichan 1 = encoder 2
         *
         * In case timestamping is enabled
         * ichan 0 = encoder 1
         * ichan 1 = timestamp encoder 1
         */
    
        /* TODO: Make suitable for encoder timestamping */
        int ivalue, index_encoder_vars;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }
        
        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }

        /* Check if ichan is 0 or 1 else exit */
        ivalue = in_Ebox_[ilink]->enc[ichan];
        
        /* MRonde (20111107): Constants changed to floating point for 32 bits */
        if ( (double) ivalue > 2147483648.0 ) {
            ivalue=(int) -(4294967296.0-ivalue);
        }
        
        /* Original software only suitable for 1 encoder channel per module
         * take into account two channels for Ebox_count0, Ebox_ifirst and Ebox_irevol
         * ichan = [0,1]
         * 
         * Memory order of variables
         * enc0[0]
         * ...
         * enc0[max_ilink]
         * enc1[0]
         * ...
         * enc1[max_ilink]
         */
        
        index_encoder_vars = ilink+ichan*MAX_NUMBER_OF_SLAVES;
        
        if (Ebox_ifirst[index_encoder_vars] == 1) {
            Ebox_count0[index_encoder_vars] =   ivalue;
            Ebox_ifirst[index_encoder_vars] =   0;
        }
        
        
        /* Correct values if not timestamping or ichan=0 */
        if((ppebox+ilink)->encoder_mode == 0 || ichan == 0){
            /* correct with initial counter value */
            ivalue=ivalue-Ebox_count0[index_encoder_vars];
            
            /* keep track of over/underflow */
            if ( (ivalue-Ebox_prev_count[index_encoder_vars])>EBOX_ENC_OVERFLOW_BOUND ) { /* assume underflow! */
                Ebox_irevol[index_encoder_vars]--;
                /* printf("Underflow \n"); */
            } else if ( (prev_count[index_encoder_vars]-ivalue)>EBOX_ENC_OVERFLOW_BOUND ) { /* assume overflow! */
                Ebox_irevol[index_encoder_vars]++;
                /* printf("Overflow \n"); */
            }
        }
        /* store counter value */
        Ebox_prev_count[ilink]=ivalue;
        
        /* MRonde (20111107): Constants changed to floating point for 32 bits and added revolution compensation to double (int32 will overflow) */
        *pvalue=(double)ivalue + Ebox_irevol[ilink]*4294967296.0;

        return EC_SUCCESS;
}





int ec_Ebox_di_read_chan(double *pvalue, int ichan, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>7) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue = ( in_Ebox_[ilink]->din & bitmask[ichan] ) >> ichan;

        *pvalue = (double) ivalue;

        return EC_SUCCESS;
}

int ec_Ebox_status_read_chan(double *pvalue, int ichan, int ilink)
{
        /*  Status
         *  The status register is used for signalling the current state.
         *  Bit 0 :		Encoder 1 index pulse triggered.
         *  Bit 1 :		Encoder 2 index pulse triggered.
         */

        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue = ( in_Ebox_[ilink]->status & bitmask[ichan] ) >> ichan;

        *pvalue = (double) ivalue;

        return EC_SUCCESS;
}



/*================================================================================
 *  Ebox functionality (Outputs)
 *================================================================================
 */
int ec_Ebox_dac_write_chan(double voltage_f, int ichan, int ilink)
{
        double  temp;
        int     voltage_i;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }

        temp = voltage_f/EBOX_DAC_RESOLUTION; /* is EBOX_DAC_RESOLUTION properly defined? */
        voltage_i = (int) temp;

        /* analog output */
        (out_Ebox_[ilink])->aout[ichan]=(int16) voltage_i;

        return 0;
}





int ec_Ebox_do_write_chan(double value, int ichan, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>7) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue=(int) value;

        switch (ivalue) {
        case 0:
            out_Ebox_[ilink]->dout = (out_Ebox_[ilink]->dout & (~bitmask[ichan]));
            break;
        case 1:
            out_Ebox_[ilink]->dout = (out_Ebox_[ilink]->dout | bitmask[ichan]);
            break;
        }

        return EC_SUCCESS;
}

int ec_Ebox_pwm_write_chan(double value, int ichan, int ilink)
{
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue=(int) value;
        
        /*TODO: Center range around 0? */
        /* Range limitation 0 - 2000 */
        if(ivalue > 2000){
            ivalue = 2000;
        } else if( ivalue < 0 ){
            ivalue = 0;
        }
        out_Ebox_[ilink]->pwmout[ichan] = ivalue;
        
        return EC_SUCCESS;
}

int ec_Ebox_control_write_chan(double value, int ichan, int ilink)
{   
        /*
         *  Control
         *  The control register is used for trigger functions and mode settings.
         *
         *  Bit 0 : 	Arms the index trigger for encoder 1. The first index pulse resets the encoder
		 *	and sets bit 0 in the status register. To retrigger write 0 and then 1 again.
         *  Bit 1 :		Arms the index trigger for encoder 2.
         */
        int ivalue;

        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }

        ivalue=(int) value;

        switch (ivalue) {
        case 0:
            out_Ebox_[ilink]->control = (out_Ebox_[ilink]->control & (~bitmask[ichan]));
            break;
        case 1:
            out_Ebox_[ilink]->control = (out_Ebox_[ilink]->control | bitmask[ichan]);
            break;
        }       

#ifdef DEBUG        
        printf("setting control register to %d \n",out_Ebox_[ilink]->control);
#endif
        
        return EC_SUCCESS;
}

int ec_Ebox_enc_reset_chan(int ichan, int ilink)
{           
        /* Performs a soft-reset of the encoders by changing count0 value */
        int index_encoder_vars;
        double encvalue=0.0;
        
        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }
        
        ec_Ebox_enc_read_chan(&encvalue ,ichan, ilink);
        
        index_encoder_vars = ilink+ichan*MAX_NUMBER_OF_SLAVES;
        /* correct with initial counter value */
        Ebox_count0[index_encoder_vars] += (int) encvalue;
        
        return EC_SUCCESS;
}

int ec_Ebox_enc_zero_chan(int ichan, int ilink)
{           
        /* Set count0 value to zero */
        int index_encoder_vars;      
        
        if ( (ilink<LINK_ID_LOW) || (ilink>LINK_ID_HIGH) || (ilink>nEbox-1) ) { return EC_ERR_INVALID_LINK_ID; }

        if ( (ichan<0) || (ichan>1) ) { return EC_ERR_INVALID_CHAN_ID; }               
        
        index_encoder_vars = ilink+ichan*MAX_NUMBER_OF_SLAVES;
        /* Set initial count to zero */
        Ebox_count0[index_encoder_vars] = 0;
        
        return EC_SUCCESS;
}



int ec_Ebox_get_ndevs(void)
{
        return nEbox;
}



int ec_receive(void)
{        
        if (ec_slave[0].state == EC_STATE_OPERATIONAL ) {    
            ec_receive_processdata(EC_TIMEOUTRET);
            return EC_SUCCESS;
        } else {
            printf("ec_receive reports: IO failed.\n");
            return EC_ERR_IO;
        }			
}

int ec_send(void)
{
           
        if (ec_slave[0].state == EC_STATE_OPERATIONAL ) {    
            ec_send_processdata();
            return EC_SUCCESS;
        } else {
            printf("ec_send reports: IO failed.\n");
            return EC_ERR_IO;
        }			
}

int ec_io(void)
{
           
        if (ec_slave[0].state == EC_STATE_OPERATIONAL ) {    
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            return EC_SUCCESS;
        } else {
            printf("ec_io reports: IO failed.\n");
            return EC_ERR_IO;
        }			
}

void ec_print_error_msg(int retval)
{
    /* Prints error message which belongs to the returnvalue */
    switch(retval){
        case EC_SUCCESS:
            break;
        case EC_ERR_INVALID_LINK_ID:
            printf("ec_error reports: invalid link_id\n");
            break;
        case EC_ERR_INVALID_CHAN_ID:
            printf("ec_error reports: invalid chan_id\n");
            break;
        case EC_ERR_INVALID_PORT_ID:
            printf("ec_error reports: invalid port_id\n");
            break;
        case EC_ERR_SOCKET:
            printf("ec_error reports: master socket bind failed.\n");
            break;
        case EC_ERR_OPSTATE:
            printf("ec_error reports: could not set slaves to OP state.\n");
            break;
        case EC_ERR_NOSLAVES:
            printf("ec_error reports: no EtherCAT slaves found\n");
            break;
        case EC_ERR_IO:
            printf("ec_error reports: i/o failed\n");            
            break;
        default:
            printf("ec_error reports: unknown error\n");
            break;
    }
}   


/* Port capability structure (set/get) */
PRIVATE int ec_count_ports(void) /* counts number of ports of a specific type */
{
    int cnt;
    
    /* MRonde, initialize at zero to prevent recounts when run multiple times */
    n.din    =   0;
    n.dout   =   0;
    n.adc    =   0;
    n.dac    =   0;
    n.enc    =   0;
    n.pwm    =   0;
    npar    =   0;

	for ( cnt=1; cnt<=ec_slavecount ;cnt++) {
        	switch (ec_slave[cnt].eep_id) {
                case EK1100_DEV_ID:
              		break;

                case EL1008_DEV_ID: /* digital in */
                        n.din=n.din+EL1008_NCHAN;       
               		break;

                case EL1018_DEV_ID: /* digital in */
                        n.din=n.din+EL1018_NCHAN;
               		break;

                case EL1014_DEV_ID: /* digital in */
                        n.din=n.din+EL1014_NCHAN;
              		break;

                case EL2004_DEV_ID: /* digital out */
                        n.dout=n.dout+EL2004_NCHAN;
              		break;

                case EL2008_DEV_ID: /* digital out */
                        n.dout=n.dout+EL2008_NCHAN;
              		break;

                case EL3102_DEV_ID: /* analog in */
                        n.adc=n.adc+EL3102_NCHAN;
               		break;
                    
                case EL3104_DEV_ID: /* analog in */
                        n.adc=n.adc+EL3104_NCHAN;
               		break;

                case EL4132_DEV_ID: /* digital out */
                        n.dac=n.dac+EL4132_NCHAN;
               		break;

                case EL4038_DEV_ID: /* digital out */
                        n.dac=n.dac+EL4038_NCHAN;
               		break;

                case EL5101_DEV_ID: /* encoder in */
                        n.enc=n.enc+EL5101_NCHAN;                                
              		break;

                case EBOX_DEV_ID: /* analog in, analog out, encoder in, digital in, digital  out, pwm out */
                case EBOX_DEV_ID2:
                        n.din=n.din+EBOX_DIN_NCHAN;
                        n.dout=n.dout+EBOX_DOUT_NCHAN;
                        n.adc=n.adc+EBOX_ADC_NCHAN;
                        n.dac=n.dac+EBOX_DAC_NCHAN;
                        n.enc=n.enc+EBOX_ENC_NCHAN;
                        n.pwm=n.pwm+EBOX_PWM_NCHAN;
                        npar++;
              		break;

                default: /* unknown device */
                        break;                    
                }                 
        }

        printf("ec_count_ports reports: port types are\n");
        if ( n.adc>0 ) { printf("   adc:  %d\n", n.adc); }
        if ( n.dac>0 ) { printf("   dac:  %d\n", n.dac); }
        if ( n.enc>0 ) { printf("   enc:  %d\n", n.enc); }
        if ( n.din>0 ) { printf("   din:  %d\n", n.din); }
        if ( n.dout>0 ) { printf("   dout: %d\n", n.dout); }
        if ( n.pwm>0 ) { printf("   pwm:  %d\n", n.pwm); }
#ifdef DEBUG
        if ( npar>0 ) { printf("   par:  %d\n", npar); }
#endif

        return EC_SUCCESS;
}

int ec_get_nadc(void)
{
        return n.adc;
}

int ec_get_ndac(void)
{
        return n.dac;
}

int ec_get_nenc(void)
{
        return n.enc;
}

int ec_get_ndin(void)
{
        return n.din;
}

int ec_get_ndout(void)
{
        return n.dout;
}

int ec_get_npwm(void)
{
        return n.pwm;
}

int ec_write_ports_config_file(FILE *fp, int mode)
{     
    switch(mode){
        default:
        case 1:
            fprintf(fp, "#define NADC %d\n", n.adc);
            fprintf(fp, "#define NDAC %d\n", n.dac);
            fprintf(fp, "#define NENC %d\n", n.enc);
            fprintf(fp, "#define NDIN %d\n", n.din);
            fprintf(fp, "#define NDOUT %d\n", n.dout);
            fprintf(fp, "#define NPWM %d\n", n.pwm);
            break;
        case 2:
            fprintf(fp, "NADC = %d;\n", n.adc);
            fprintf(fp, "NDAC = %d;\n", n.dac);
            fprintf(fp, "NENC = %d;\n", n.enc);
            fprintf(fp, "NDIN = %d;\n", n.din);
            fprintf(fp, "NDOUT = %d;\n", n.dout);
            fprintf(fp, "NPWM = %d;\n", n.pwm);
            break;
    }
            
    
    return EC_SUCCESS;
}


PRIVATE int ec_Ebox_initialize_parameters(int link_id, int slave_id)
{
    /* Initialize the parameters in this structure at startup */
    int os; 
    
    (ppebox+link_id)->link_id   =   link_id;
    (ppebox+link_id)->slave_id  =   slave_id;
    
    /* Read hardware and software version */   
    os = (int) sizeof((ppebox+link_id)->hw_ver);
    /* printf("to read %d bytes\n",os); */
    ec_SDOread(slave_id,0x1009,00,FALSE,&os,(ppebox+link_id)->hw_ver,EC_TIMEOUTRXM);    
    /* printf("read %d bytes\n",os); */
    
    os = (int) sizeof((ppebox+link_id)->sw_ver);
    /* printf("to read %d bytes\n",os); */
    ec_SDOread(slave_id,0x100a,00,FALSE,&os,(ppebox+link_id)->sw_ver,EC_TIMEOUTRXM);   
    /* printf("read %d bytes\n",os); */
    
#ifdef DEBUG    
    printf("E/BOX hardware version = %s\n",(ppebox+link_id)->hw_ver);
    printf("E/BOX software version = %s\n",(ppebox+link_id)->sw_ver);    
#endif    
    if(strcmp((ppebox+link_id)->hw_ver,"1.2")==0){
        printf("Display present\n");
    }
    
    /* Read summation filter of Ain[1] */
    os = (int) sizeof((ppebox+link_id)->filter_ain[0]);
    ec_SDOread(slave_id,0x8001,01,FALSE,&os,&(ppebox+link_id)->filter_ain[0],EC_TIMEOUTRXM);
    
    /* Read summation filter of Ain[2] */
    os = (int) sizeof((ppebox+link_id)->filter_ain[1]);
    ec_SDOread(slave_id,0x8001,02,FALSE,&os,&(ppebox+link_id)->filter_ain[1],EC_TIMEOUTRXM);
    
    
    memcpy((ppebox+link_id)->textline1,"",16);
    memcpy((ppebox+link_id)->textline2,"",16);
    memcpy((ppebox+link_id)->textline3,"",16);
    memcpy((ppebox+link_id)->hw_ver,"",6);
    memcpy((ppebox+link_id)->sw_ver,"",6);
   
    return EC_SUCCESS;
}

int ec_Ebox_get_parameters(int link_id, ppar_ebox p)
{   
    /* Copy the contents of internal parameter structure into p */
    int os;
    
    if ( link_id==(ppebox+link_id)->link_id ) {
        
        /* Read summation filter of Ain[1] */
        os = sizeof((ppebox+link_id)->filter_ain[0]);
        ec_SDOread((ppebox+link_id)->slave_id,0x8001,01,FALSE,&os,&(ppebox+link_id)->filter_ain[0],EC_TIMEOUTRXM);        
        /* Read summation filter of Ain[2] */
        os = sizeof((ppebox+link_id)->filter_ain[1]);
        ec_SDOread((ppebox+link_id)->slave_id,0x8001,02,FALSE,&os,&(ppebox+link_id)->filter_ain[1],EC_TIMEOUTRXM);        
        /* Read encoder mode */
        os = sizeof((ppebox+link_id)->encoder_mode);
        ec_SDOread((ppebox+link_id)->slave_id,0x8001,03,FALSE,&os,&(ppebox+link_id)->encoder_mode,EC_TIMEOUTRXM);  
        
        p->link_id = (ppebox+link_id)->link_id;
        p->slave_id = (ppebox+link_id)->slave_id;        
        p->filter_ain[0] = (ppebox+link_id)->filter_ain[0];
        p->filter_ain[1] = (ppebox+link_id)->filter_ain[1];
        p->encoder_mode  = (ppebox+link_id)->encoder_mode;
        
        memcpy(p->hw_ver,(ppebox+link_id)->hw_ver,6);
        memcpy(p->sw_ver,(ppebox+link_id)->sw_ver,6);        
    } else {
        return EC_ERR_INVALID_LINK_ID;
    }
    
    return EC_SUCCESS;
}

int ec_Ebox_set_parameters(int link_id, ppar_ebox p)
{
    /* Copy the contents of p to internal parameter structure and send to ebox */
    int os; 
    
    if ( link_id==(ppebox+link_id)->link_id ) {
        
        /* Set summation filter of Ain[1] */
        os = sizeof(p->filter_ain[0]);
        ec_SDOwrite(p->slave_id,0x8001,01,FALSE,os,&p->filter_ain[0],EC_TIMEOUTRXM);
        /* Set summation filter of Ain[2] */
        os = sizeof(p->filter_ain[1]);
        ec_SDOwrite(p->slave_id,0x8001,02,FALSE,os,&p->filter_ain[1],EC_TIMEOUTRXM);             
        /* Set encoder mode */
        os = sizeof(p->encoder_mode);
        ec_SDOwrite(p->slave_id,0x8001,03,FALSE,os,&p->encoder_mode,EC_TIMEOUTRXM); 
        
        /* Display */
        os = 16;
        ec_SDOwrite(p->slave_id,0x8002,1,FALSE,os,&p->textline1,EC_TIMEOUTRXM);  
        ec_SDOwrite(p->slave_id,0x8002,2,FALSE,os,&p->textline2,EC_TIMEOUTRXM);
        ec_SDOwrite(p->slave_id,0x8002,3,FALSE,os,&p->textline3,EC_TIMEOUTRXM);
        
        /* Copy set parameters to internal structure */
        (ppebox+link_id)->filter_ain[0] = p->filter_ain[0];
        (ppebox+link_id)->filter_ain[1] = p->filter_ain[1];
        (ppebox+link_id)->encoder_mode  = p->encoder_mode;
        memcpy((ppebox+link_id)->hw_ver,p->hw_ver,6);
        memcpy((ppebox+link_id)->sw_ver,p->sw_ver,6);           
#ifdef DEBUG        
        printf("ec_Ebox_set_parameters reports: setting adc summation filter parameters = [%d, %d] \n", p->filter_ain[0], p->filter_ain[1]);
        printf("ec_Ebox_set_parameters reports: setting encoder mode = %d \n",p->encoder_mode);
#endif
    } else {
        return EC_ERR_INVALID_LINK_ID;
    }
    
    return EC_SUCCESS;
}

PRIVATE int ec_set_adc_port_capabilities(int port_id, int link_id, int chan_id, double res, double range, int enabled, int dev_id, char *dev_str)
{
        (ppadc+port_id)->port_id=port_id;
        (ppadc+port_id)->link_id=link_id;
        (ppadc+port_id)->chan_id=chan_id;
        (ppadc+port_id)->resolution=res;
        (ppadc+port_id)->range=range;
        (ppadc+port_id)->enabled=enabled;
        (ppadc+port_id)->dev_id=dev_id;
        strcpy((ppadc+port_id)->dev_str, dev_str);

        return EC_SUCCESS;
}





int ec_get_adc_port_capabilities(int port_id, pport_adc p)
{
        if ( port_id==(ppadc+port_id)->port_id ) {
                p->port_id=(ppadc+port_id)->port_id;
                p->link_id=(ppadc+port_id)->link_id;
                p->chan_id=(ppadc+port_id)->chan_id;
                p->resolution=(ppadc+port_id)->resolution;
                p->range=(ppadc+port_id)->range;
                p->enabled=(ppadc+port_id)->enabled;
                p->dev_id=(ppadc+port_id)->dev_id;
                strcpy(p->dev_str,(ppadc+port_id)->dev_str);
                
                return EC_SUCCESS;
        } else {
                return EC_ERR_INVALID_PORT_ID;
        }
}





PRIVATE int ec_set_dac_port_capabilities(int port_id, int link_id, int chan_id, double res, double range, int enabled, int dev_id, char *dev_str)
{
        (ppdac+port_id)->port_id=port_id;
        (ppdac+port_id)->link_id=link_id;
        (ppdac+port_id)->chan_id=chan_id;
        (ppdac+port_id)->resolution=res;
        (ppdac+port_id)->range=range;
        (ppdac+port_id)->enabled=enabled;
        (ppdac+port_id)->dev_id=dev_id;
        strcpy((ppdac+port_id)->dev_str, dev_str);        

        return EC_SUCCESS;
}





int ec_get_dac_port_capabilities(int port_id, pport_dac p)
{
        if ( port_id==(ppdac+port_id)->port_id ) {
                p->port_id=(ppdac+port_id)->port_id;
                p->link_id=(ppdac+port_id)->link_id;
                p->chan_id=(ppdac+port_id)->chan_id;
                p->resolution=(ppdac+port_id)->resolution;
                p->range=(ppdac+port_id)->range;
                p->enabled=(ppdac+port_id)->enabled;
                p->dev_id=(ppdac+port_id)->dev_id;
                strcpy(p->dev_str,(ppdac+port_id)->dev_str);
                
                return EC_SUCCESS;
        } else {
                return EC_ERR_INVALID_PORT_ID;
        }
}





PRIVATE int ec_set_enc_port_capabilities(int port_id, int link_id, int chan_id, int enabled, int dev_id, char *dev_str)
{
        (ppenc+port_id)->port_id=port_id;
        (ppenc+port_id)->link_id=link_id;
        (ppenc+port_id)->chan_id=chan_id;
        (ppenc+port_id)->enabled=enabled;
        (ppenc+port_id)->dev_id=dev_id;
        strcpy((ppenc+port_id)->dev_str, dev_str);         

        return EC_SUCCESS;
}





int ec_get_enc_port_capabilities(int port_id, pport_enc p)
{
        if ( port_id==(ppenc+port_id)->port_id ) {
                p->port_id=(ppenc+port_id)->port_id;
                p->link_id=(ppenc+port_id)->link_id;
                p->chan_id=(ppenc+port_id)->chan_id;
                p->enabled=(ppenc+port_id)->enabled;
                p->dev_id=(ppenc+port_id)->dev_id;
                strcpy(p->dev_str,(ppenc+port_id)->dev_str);
                
                return EC_SUCCESS;
        } else {
                return EC_ERR_INVALID_PORT_ID;
        }
}



PRIVATE int ec_set_din_port_capabilities(int port_id, int link_id, int chan_id, int enabled, int dev_id, char *dev_str)
{
        (ppdin+port_id)->port_id=port_id;
        (ppdin+port_id)->link_id=link_id;
        (ppdin+port_id)->chan_id=chan_id;
        (ppdin+port_id)->enabled=enabled;
        (ppdin+port_id)->dev_id=dev_id;
        strcpy((ppdin+port_id)->dev_str, dev_str);            

        return EC_SUCCESS;
}

int ec_get_din_port_capabilities(int port_id, pport_din p)
{
        if ( port_id==(ppdin+port_id)->port_id ) {
                p->port_id=(ppdin+port_id)->port_id;
                p->link_id=(ppdin+port_id)->link_id;
                p->chan_id=(ppdin+port_id)->chan_id;
                p->enabled=(ppdin+port_id)->enabled;
                p->dev_id=(ppdin+port_id)->dev_id;
                strcpy(p->dev_str,(ppdin+port_id)->dev_str);    
                
                return EC_SUCCESS;
        } else {
                return EC_ERR_INVALID_PORT_ID;
        }
}


PRIVATE int ec_set_dout_port_capabilities(int port_id, int link_id, int chan_id, int enabled, int dev_id, char *dev_str)
{
        (ppdout+port_id)->port_id=port_id;
        (ppdout+port_id)->link_id=link_id;
        (ppdout+port_id)->chan_id=chan_id;
        (ppdout+port_id)->enabled=enabled;
        (ppdout+port_id)->dev_id=dev_id;
        strcpy((ppdout+port_id)->dev_str, dev_str);          

        return EC_SUCCESS;
}

int ec_get_dout_port_capabilities(int port_id, pport_dout p)
{
        if ( port_id==(ppdout+port_id)->port_id ) {
                p->port_id=(ppdout+port_id)->port_id;
                p->link_id=(ppdout+port_id)->link_id;
                p->chan_id=(ppdout+port_id)->chan_id;
                p->enabled=(ppdout+port_id)->enabled;
                p->dev_id=(ppdout+port_id)->dev_id;
                strcpy(p->dev_str,(ppdout+port_id)->dev_str);  
                
                return EC_SUCCESS;
        } else {
                return EC_ERR_INVALID_PORT_ID;
        }
}

PRIVATE int ec_set_pwm_port_capabilities(int port_id, int link_id, int chan_id, int enabled, int dev_id, char *dev_str)
{
        (pppwm+port_id)->port_id=port_id;
        (pppwm+port_id)->link_id=link_id;
        (pppwm+port_id)->chan_id=chan_id;
        (pppwm+port_id)->enabled=enabled;
        (pppwm+port_id)->dev_id=dev_id;
        strcpy((pppwm+port_id)->dev_str, dev_str);            

        return EC_SUCCESS;
}

int ec_get_pwm_port_capabilities(int port_id, pport_pwm p)
{
        if ( port_id==(pppwm+port_id)->port_id ) {
                p->port_id=(pppwm+port_id)->port_id;
                p->link_id=(pppwm+port_id)->link_id;
                p->chan_id=(pppwm+port_id)->chan_id;
                p->enabled=(pppwm+port_id)->enabled;
                p->dev_id=(pppwm+port_id)->dev_id;
                strcpy(p->dev_str,(pppwm+port_id)->dev_str);  
                
                return EC_SUCCESS;
        } else {
                return EC_ERR_INVALID_PORT_ID;
        }
}




/* Port capability structure (read/write) */
int ec_adc_read_chan(double *pvalue, int port_id)
{
        int iret;
        double clipval;

        switch ( (ppadc+port_id)->dev_id ) {
        case EL3102_DEV_ID:
                iret=ec_EL3102_adc_read_chan(pvalue, (ppadc+port_id)->chan_id, (ppadc+port_id)->link_id);
                break;
        case EL3104_DEV_ID:
                iret=ec_EL3104_adc_read_chan(pvalue, (ppadc+port_id)->chan_id, (ppadc+port_id)->link_id);
                break;                
        case EBOX_DEV_ID:
                iret=ec_Ebox_adc_read_chan(pvalue, (ppadc+port_id)->chan_id, (ppadc+port_id)->link_id);
                break;
        default:
                return EC_ERR_INVALID_PORT_ID;
        }

        switch (iret) {
        case EC_SUCCESS:
/*	        check for clipping */
	        (ppadc+port_id)->clipped=0;
                /* Changed clipval depending on resolution */
                clipval=(ppadc+port_id)->range/2.0-(ppadc+port_id)->resolution;
	        if ( absval(*pvalue)>clipval ) {
		        (ppadc+port_id)->clipped=1;
		        *pvalue=signval(*pvalue)*clipval;
	        }
                return iret;
        default:
                return iret;
        }
}





int ec_adc_get_clipped_port(int* pclipped, int port_id)
{
        /* Check if port_id is acceptable, otherwise return EC_ERR_INVALID_PORT_ID */
        /* TODO: Check why clipped value is nonzero for invalid port_id since memory is set to zero*/
        if(port_id < n.adc){
            *pclipped=(ppadc+port_id)->clipped;
            /* DEBUG
             * printf("eclib reports: port_id=%d \t clipped=%d\n",port_id,(ppadc+port_id)->clipped);
             */
            return EC_SUCCESS;
        } else {
            *pclipped = 0;
            return EC_ERR_INVALID_PORT_ID;
        }
        
}

int ec_adc_get_state(int* state, int port_id)
{
        if(port_id < n.adc){
            *state=(ppadc+port_id)->enabled;
            return EC_SUCCESS;
        } else {
            *state = 0;
            return EC_ERR_INVALID_PORT_ID;
        }
    
}

int ec_adc_set_state(int state, int port_id)
{
        if(port_id < n.adc){
            if((state<0) || (state>1) ){
                return EC_ERR_INVALID_STATE;
            } else {
                (ppadc+port_id)->enabled=state;
            }
            return EC_SUCCESS;
        } else {
            state = 0;
            return EC_ERR_INVALID_PORT_ID;
        }
    
}




int ec_dac_write_chan(double voltage_f, int port_id)
{
        switch ( (ppdac+port_id)->dev_id ) {
        case EL4132_DEV_ID:
                return ec_EL4132_dac_write_chan(voltage_f, (ppdac+port_id)->chan_id, (ppdac+port_id)->link_id);
        case EL4038_DEV_ID:
                return ec_EL4038_dac_write_chan(voltage_f, (ppdac+port_id)->chan_id, (ppdac+port_id)->link_id);
        case EBOX_DEV_ID:
                return ec_Ebox_dac_write_chan(voltage_f, (ppdac+port_id)->chan_id, (ppdac+port_id)->link_id);
        default:
                return EC_ERR_INVALID_PORT_ID;
        }
}





int ec_enc_read_chan(double *pvalue, int port_id)
{
        switch ( (ppenc+port_id)->dev_id ) {
        case EL5101_DEV_ID:
                return ec_EL5101_enc_read_chan(pvalue, (ppenc+port_id)->link_id); /* only one channel */
        case EBOX_DEV_ID:
                return ec_Ebox_enc_read_chan(pvalue, (ppenc+port_id)->chan_id, (ppenc+port_id)->link_id);
        default:
                return EC_ERR_INVALID_PORT_ID;
        }
}

int ec_enc_get_state(int* state, int port_id)
{
        if(port_id < n.enc){
            *state=(ppenc+port_id)->enabled;
            return EC_SUCCESS;
        } else {
            *state = 0;
            return EC_ERR_INVALID_PORT_ID;
        }
    
}

int ec_enc_set_state(int state, int port_id)
{
        if(port_id < n.enc){
            if((state<0) || (state>1) ){
                return EC_ERR_INVALID_STATE;
            } else {
                (ppenc+port_id)->enabled=state;
            }
            return EC_SUCCESS;
        } else {
            state = 0;
            return EC_ERR_INVALID_PORT_ID;
        }
    
}

int ec_din_read_chan(double *pvalue, int port_id)
{
        switch ( (ppdin+port_id)->dev_id ) {           
        case EL1008_DEV_ID:  
            return ec_EL1008_di_read_chan(pvalue, (ppdin+port_id)->chan_id, (ppdin+port_id)->link_id);
        case EL1018_DEV_ID:  
            return ec_EL1018_di_read_chan(pvalue, (ppdin+port_id)->chan_id, (ppdin+port_id)->link_id);            
        case EL1014_DEV_ID:  
            return ec_EL1014_di_read_chan(pvalue, (ppdin+port_id)->chan_id, (ppdin+port_id)->link_id);                                    
        case EBOX_DEV_ID:
            return ec_Ebox_di_read_chan(pvalue, (ppdin+port_id)->chan_id, (ppdin+port_id)->link_id);
        default:
                return EC_ERR_INVALID_PORT_ID;
        }
}        

int ec_dout_write_chan(double pvalue, int port_id)
{
        switch ( (ppdout+port_id)->dev_id ) {
        case EL2004_DEV_ID:
                return ec_EL2004_do_write_chan(pvalue, (ppdac+port_id)->chan_id, (ppdac+port_id)->link_id);
        case EL2008_DEV_ID:            
                return ec_EL2008_do_write_chan(pvalue, (ppdac+port_id)->chan_id, (ppdac+port_id)->link_id);            
        case EBOX_DEV_ID:
                return ec_Ebox_do_write_chan(pvalue, (ppdac+port_id)->chan_id, (ppdac+port_id)->link_id);
        default:
                return EC_ERR_INVALID_PORT_ID;
        }
}

int ec_pwm_write_chan(double pvalue, int port_id)
{
        switch ( (ppdout+port_id)->dev_id ) {  
        case EBOX_DEV_ID:
                return ec_Ebox_pwm_write_chan(pvalue, (ppdac+port_id)->chan_id, (ppdac+port_id)->link_id);
        default:
                return EC_ERR_INVALID_PORT_ID;
        }
}



int ec_start(char* netif)
{
	int cnt, i;
        int iadc=0;
        int idac=0;
        int ienc=0;
        int idin=0;
        int idout=0;
        int ipwm=0;

	printf("ec_start reports: opening EtherCAT communication.\n");
    
   	/* initialise SOEM, bind socket to ifname */
	if (ec_init(netif)) {	

       		printf("ec_start reports: master socket bind on %s succeeded.\n",netif);
        
        	/* bit masks voor digital I/O */
        	bitmask[0]=0x01;
        	bitmask[1]=0x02;
        	bitmask[2]=0x04;
        	bitmask[3]=0x08;
        	bitmask[4]=0x10;
        	bitmask[5]=0x20;
        	bitmask[6]=0x40;
        	bitmask[7]=0x80;

        	/* initialize encoder data */
        	for (i=0; i<MAX_NUMBER_OF_SLAVES; i++) {
            		count0[i]=0;
            		ifirst[i]=1;
                    ilatch[i]=1;    /* If 1 latch variable needs to be reset in control register*/
            		irevol[i]=0;
            		prev_count[i]=0;
        	}       

        	/* initialize encoder data of E-Box (2 encoder channels per box) */
        	for (i=0; i<2*MAX_NUMBER_OF_SLAVES; i++) {
            		Ebox_count0[i]=0;
            		Ebox_ifirst[i]=1;
            		Ebox_irevol[i]=0;
            		Ebox_prev_count[i]=0;
        	}    
               
		/* find and auto-config slaves */
		if ( ec_config(FALSE, &IOmap) > 0 ) {
			printf("ec_start reports: %d slaves found and configured.\n",ec_slavecount);
			/* wait for all slaves to reach SAFE_OP state */
			ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);
			/* read individual slave state and store in ec_slave[] */
			ec_readstate();

/*                      scan number of ports of specific type */
                        ec_count_ports();

/*                      allocate memory for ports of specific type */
                        ppadc=(pport_adc) calloc(n.adc, sizeof(port_adc));
                        ppdac=(pport_dac) calloc(n.dac, sizeof(port_dac));
                        ppenc=(pport_enc) calloc(n.enc, sizeof(port_enc));
                        ppdin=(pport_din) calloc(n.din, sizeof(port_din));
                        ppdout=(pport_dout) calloc(n.dout, sizeof(port_dout));
                        pppwm=(pport_pwm) calloc(n.pwm, sizeof(port_pwm));
                        ppebox=(ppar_ebox) calloc(npar, sizeof(par_ebox));
                        
                        /* Check if all the memory is allocated */
                        if(ppadc==NULL || ppdac==NULL || ppenc== NULL || ppdin==NULL || ppdout==NULL || pppwm==NULL || ppebox==NULL){
                            printf("ec_start reports: memory allocation error\n");
                            return EC_ERR_MALLOC;
                        }

			for(cnt = 1; cnt <= ec_slavecount ; cnt++)
			{
				printf("   slave: %d   name: %s   output size: %3d bits   input size: %3d bits   state: %2d\n",
                                       cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits, ec_slave[cnt].state);

                		/*slave configuration*/

               			switch (ec_slave[cnt].eep_id) {
                                case EK1100_DEV_ID:

                                	nEK1100++;
                          		break;

                                case EL1008_DEV_ID:
                                
                                	in_EL1008_[nEL1008] = (in_EL1008t*) ec_slave[cnt].inputs;
                                    
                                        for (i=0; i<EL1008_NCHAN; i++) {
                                                /* din, chanel i */
                                                ec_set_din_port_capabilities(idin, nEL1008, i, 1, EL1008_DEV_ID, EL1008_DEV_STR);
                                                idin++;
                                        }
                                    
                                	nEL1008++;    
                          		break;

                                case EL1018_DEV_ID:
                                
                                	in_EL1018_[nEL1018] = (in_EL1018t*) ec_slave[cnt].inputs;
                                    
                                        for (i=0; i<EL1018_NCHAN; i++) {
                                                /* din, chanel i */
                                                ec_set_din_port_capabilities(idin, nEL1018, i, 1, EL1018_DEV_ID, EL1018_DEV_STR);
                                                idin++;
                                        }
                                    
                                	nEL1018++;
                          		break;

                                case EL1014_DEV_ID:
                                
                                	in_EL1014_[nEL1014] = (in_EL1014t*) ec_slave[cnt].inputs;
                                    
                                        for (i=0; i<EL1014_NCHAN; i++) {
                                                /* din, chanel i */
                                                ec_set_din_port_capabilities(idin, nEL1014, i, 1, EL1014_DEV_ID, EL1014_DEV_STR);
                                                idin++;
                                        }
                                    
                                	nEL1014++;
                          		break;
	
                                case EL2004_DEV_ID:
                                 
                                	out_EL2004_[nEL2004] = (out_EL2004t*) ec_slave[cnt].outputs;
                                    
                                        for (i=0; i<EL2004_NCHAN; i++) {
                                                /* dout, chanel i */
                                                ec_set_dout_port_capabilities(idout, nEL2004, i, 1, EL2004_DEV_ID, EL2004_DEV_STR);
                                                idout++;
                                        }
                                    
                                	nEL2004++;
                          		break;

                                case EL2008_DEV_ID:
                               
                                	out_EL2008_[nEL2008] = (out_EL2008t*) ec_slave[cnt].outputs;  

                                        for (i=0; i<EL2008_NCHAN; i++) {
                                                /* dout, chanel i */
                                                ec_set_dout_port_capabilities(idout, nEL2008, i, 1, EL2008_DEV_ID, EL2008_DEV_STR);
                                                idout++;
                                        }
                                    
                                	nEL2008++;
                          		break;

                                case EL3102_DEV_ID:
                                
                                	in_EL3102_[nEL3102] = (in_EL3102t*) ec_slave[cnt].inputs;

                                        for (i=0; i<EL3102_NCHAN; i++) {
                                                /* adc, channel i */
                                                ec_set_adc_port_capabilities(iadc, nEL3102, i, EL3102_RESOLUTION, EL3102_RANGE, 1, EL3102_DEV_ID, EL3102_DEV_STR);
                                                iadc++;
                                        }

                                	nEL3102++;
                          		break;
                                
                                case EL3104_DEV_ID:
                                    in_EL3104_[nEL3104] = (in_EL3104t*) ec_slave[cnt].inputs;
                                    
                                    for (i=0; i<EL3104_NCHAN; i++) {
                                                /* adc, channel i */
                                                ec_set_adc_port_capabilities(iadc, nEL3104, i, EL3104_RESOLUTION, EL3104_RANGE, 1, EL3104_DEV_ID, EL3104_DEV_STR);
                                                iadc++;
                                    }
                                    
                                    nEL3104++;
                                break;
                                    

                                case EL4132_DEV_ID:
                                
                                	out_EL4132_[nEL4132] = (out_EL4132t*) ec_slave[cnt].outputs;

                                        for (i=0; i<EL4132_NCHAN; i++) {
                                                /* dac, channel i */
                                                ec_set_dac_port_capabilities(idac, nEL4132, i, EL4132_DAC_RESOLUTION, EL4132_DAC_RANGE, 1, EL4132_DEV_ID, EL4132_DEV_STR);
                                                idac++;
                                        }

                                	nEL4132++;
                          		break;

                                case EL4038_DEV_ID:
                                
                                	out_EL4038_[nEL4038] = (out_EL4038t*) ec_slave[cnt].outputs;

                                        for (i=0; i<EL4038_NCHAN; i++) {
                                                /* dac, channel i */
                                                ec_set_dac_port_capabilities(idac, nEL4038, i, EL4038_DAC_RESOLUTION, EL4038_DAC_RANGE, 1, EL4038_DEV_ID, EL4038_DEV_STR);
                                                idac++;
                                        }

                                	nEL4038++;
                          		break;

                                case EL5101_DEV_ID:
                                
                                	out_EL5101_[nEL5101] = (out_EL5101t*) ec_slave[cnt].outputs;
                                	in_EL5101_[nEL5101] = (in_EL5101t*) ec_slave[cnt].inputs;

                                        for (i=0; i<EL5101_NCHAN; i++) {
                                                /* enc, channel i */
                                                ec_set_enc_port_capabilities(ienc, nEL5101, i, 1, EL5101_DEV_ID, EL5101_DEV_STR);
                                                ienc++;
                                        }

                                	nEL5101++;
                          		break;
			
                                case EBOX_DEV_ID:
                                case EBOX_DEV_ID2:
                                	out_Ebox_[nEbox]  = (out_Eboxt*) ec_slave[cnt].outputs;
                                	in_Ebox_[nEbox]   = (in_Eboxt*) ec_slave[cnt].inputs;

                                        for (i=0; i<EBOX_ADC_NCHAN; i++) {
                                                /* adc, channel i */
                                                ec_set_adc_port_capabilities(iadc, nEbox, i, EBOX_ADC_RESOLUTION, EBOX_ADC_RANGE, 1, EBOX_DEV_ID, EBOX_DEV_STR);
                                                iadc++;
                                        }

                                        for (i=0; i<EBOX_DAC_NCHAN; i++) {
                                                /* dac, channel i */
                                                ec_set_dac_port_capabilities(idac, nEbox, i, EBOX_DAC_RESOLUTION, EBOX_DAC_RANGE, 1, EBOX_DEV_ID, EBOX_DEV_STR);
                                                idac++;
                                        }

                                        for (i=0; i<EBOX_ENC_NCHAN; i++) {
                                                /* enc, channel i */
                                                ec_set_enc_port_capabilities(ienc, nEbox, i, 1, EBOX_DEV_ID, EBOX_DEV_STR);
                                                ienc++;
                                        }
                                    
                                        for (i=0; i<EBOX_DIN_NCHAN; i++) {
                                                /* din, chanel i */
                                                ec_set_din_port_capabilities(idin, nEbox, i, 1, EBOX_DEV_ID, EBOX_DEV_STR);
                                                idin++;
                                        }
                                        
                                        for (i=0; i<EBOX_DOUT_NCHAN; i++) {
                                                /* dout, chanel i */
                                                ec_set_dout_port_capabilities(idout, nEbox, i, 1, EBOX_DEV_ID, EBOX_DEV_STR);
                                                idout++;
                                        }
                                    
                                        for (i=0; i<EBOX_PWM_NCHAN; i++) {
                                                /* pwm, chanel i */
                                                ec_set_pwm_port_capabilities(ipwm, nEbox, i, 1, EBOX_DEV_ID, EBOX_DEV_STR);
                                                ipwm++;
                                        }
                                        #ifdef DEBUG
                                            printf("Reading ebox parameters\n");
                                        #endif
                                        ec_Ebox_initialize_parameters(nEbox,cnt);

                                	nEbox++;
                          		break;
                          
                                default:
                                	printf("ec_start reports: unregistered slave detected (ID = 0x%x).\n", (unsigned int) ec_slave[cnt].eep_id);
                          		break;                    
               			}                 
			}
            
        	} else {
                	printf("ec_start reports: no EtherCAT slaves found. Are slaves connected to %s port?\n",netif);
                	return EC_ERR_NOSLAVES;
               	}
        } else {
                printf("ec_start reports: master socket bind on %s failed.\n",netif);
                return EC_ERR_SOCKET;
        }

/*      set slaves to operational state */
        if ( ec_setop()==EC_ERR_OPSTATE ) {
                printf("ec_start reports: could not set slaves to OP state.\n");
                return EC_ERR_OPSTATE;
        }		
   
        return EC_SUCCESS;
}





PRIVATE int ec_setop(void)
{
        /* AKETELS do one process data cycle before going to operational */
        /* slaves will check valid process data before going to OP otherwise */
        /* they will time out and go to SAFE_OP+ERROR state */
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);

        ec_slave[0].state = EC_STATE_OPERATIONAL;
        /* request OP state for all slaves */
        ec_writestate(0);
        /* wait for all slaves to reach OP state */
        ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);
            
        if (ec_slave[0].state == EC_STATE_OPERATIONAL ) {
    	        return EC_SUCCESS; 
        } else {
    	        printf("ec_setop reports: not all slaves reached operational state.\n");
                ec_slave[0].state = EC_STATE_SAFE_OP;
                /* request safe state for all slaves */
                ec_writestate(0);
                return EC_ERR_OPSTATE;
        }		          
}





int ec_stop(void)
{
        /* clear memory */
        free(ppadc);
        free(ppdac);
        free(ppenc);
        free(ppdin);
        free(ppdout);
        free(pppwm);
        free(ppebox);

        printf("ec_stop reports: request safe operational state for all slaves.\n");
	ec_slave[0].state = EC_STATE_SAFE_OP;
	/* request SAFE_OP state for all slaves */
	ec_writestate(0);
		
	printf("ec_stop reports: end SOEM, close socket.\n");
	/* stop SOEM, close socket */
	ec_close();

        return EC_SUCCESS;
}

