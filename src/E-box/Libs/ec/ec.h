
#define MAX_NUMBER_OF_SLAVES    25

#define LINK_ID_LOW             0                               /* link_id starts at 0 */
#define LINK_ID_HIGH            (MAX_NUMBER_OF_SLAVES-1)

#define EBOX_ENC_OVERFLOW_BOUND 1000000000

#define EC_SUCCESS               0
#define EC_ERR_SOCKET           -1
#define EC_ERR_OPSTATE          -2
#define EC_ERR_NOSLAVES         -3
#define EC_ERR_INVALID_LINK_ID  -4
#define EC_ERR_INVALID_CHAN_ID  -5
#define EC_ERR_IO               -6
#define EC_ERR_INVALID_PORT_ID  -7
#define EC_ERR_MALLOC           -8
#define EC_ERR_INVALID_STATE    -9

/* EK1100 */
#define EK1100_DEV_ID           0x044c2c52

/* EL1008 */
#define EL1008_NCHAN            8
#define EL1008_DEV_ID           0x03f03052
#define EL1008_DEV_STR          "EL1008"

/* EL1018 */
#define EL1018_NCHAN            8
#define EL1018_DEV_ID           0x03fa3052
#define EL1018_DEV_STR          "EL1018"

/* EL1014 */
#define EL1014_NCHAN            4
#define EL1014_DEV_ID           0x03f63052
#define EL1014_DEV_STR          "EL1014"

/* EL2004 */
#define EL2004_NCHAN            4
#define EL2004_DEV_ID           0x7d43052
#define EL2004_DEV_STR          "EL2004"

/* EL2008 */
#define EL2008_NCHAN            8
#define EL2008_DEV_ID           0x7d83052
#define EL2008_DEV_STR          "EL2008"

/* EL5101 */
#define EL5101_NCHAN            1
#define EL5101_DEV_ID           0x13ed3052
#define EL5101_DEV_STR          "EL5101"

/* EL3102 */
#define EL3102_RESOLUTION       (20.0/65535.0)
#define EL3102_RANGE            20.0
#define EL3102_NCHAN            2
#define EL3102_DEV_ID           0x0c1e3052
#define EL3102_DEV_STR          "EL3102"

/* EL3104 */
#define EL3104_RESOLUTION       (20.0/65535.0)
#define EL3104_RANGE            20.0
#define EL3104_NCHAN            4
#define EL3104_DEV_ID           0x0c203052
#define EL3104_DEV_STR          "EL3104"

/* EL4132 */
#define EL4132_DAC_RESOLUTION   (20.0/65535.0)
#define EL4132_DAC_RANGE        20.0
#define EL4132_NCHAN            2
#define EL4132_DEV_ID           0x10243052
#define EL4132_DEV_STR          "EL4132"

/* EL4038 */
#define EL4038_DAC_RESOLUTION   (20.0/65535.0)
#define EL4038_DAC_RANGE        20.0
#define EL4038_NCHAN            8
#define EL4038_DEV_ID           0x0fc63052
#define EL4038_DEV_STR          "EL4038"

/* EBOX */
#define EBOX_ADC_RESOLUTION     (20.0/200000.0)
#define EBOX_ADC_RANGE          20.0
#define EBOX_ADC_NCHAN          2
#define EBOX_DAC_RESOLUTION     (20.0/65535.0)
#define EBOX_DAC_RANGE          20.0
#define EBOX_DAC_NCHAN          2
#define EBOX_ENC_NCHAN          2
#define EBOX_DIN_NCHAN          8
#define EBOX_DOUT_NCHAN         8
#define EBOX_PWM_NCHAN          2
#define EBOX_DEV_ID             0x01107001
#define EBOX_DEV_ID2            0x00904004
#define EBOX_DEV_STR            "E/BOX"

/*Only define when ethercattype.h is not included*/
#ifndef _EC_TYPE_H
    typedef unsigned char       uint8;
#endif




typedef struct tag_port_adc
{
        int port_id;            /**< logical adc port_id in stack */
        int link_id;            /**< link_id in stack */
        int chan_id;            /**< chan_id in device */
        double resolution;      /**< in V */
        double range;           /**< in V */
        int enabled;            /**< 0 (disabled) or 1 (enabled) */
        int dev_id;             /**< id of slave */
        char dev_str[10];       /**< name of slave */
        int clipped;            /**< 0 (not clipped) or 1 (clipped) */
} port_adc, *pport_adc;





typedef struct tag_port_dac
{
        int port_id;            /**< logical dac port_id in stack */
        int link_id;            /**< link_id in stack */
        int chan_id;            /**< chan_id in device */
        double resolution;      /**< in V */
        double range;           /**< in V */
        int enabled;            /**< 0 (disabled) or 1 (enabled) */
        int dev_id;             /**< id of slave */
        char dev_str[10];       /**< name of slave */
} port_dac, *pport_dac;





typedef struct tag_port_enc
{
        int port_id;            /**< logical enc port_id in stack */
        int link_id;            /**< link_id in stack */
        int chan_id;            /**< chan_id in device */
        int enabled;            /**< 0 (disabled) or 1 (enabled) */
        int dev_id;             /**< id of slave */
        char dev_str[10];       /**< name of slave */
} port_enc, *pport_enc;





typedef struct tag_port_din
{
        int port_id;            /**< logical din port_id in stack */
        int link_id;            /**< link_id in stack */
        int chan_id;            /**< chan_id in device */
        int enabled;            /**< 0 (disabled) or 1 (enabled) */
        int dev_id;             /**< id of slave */
        char dev_str[10];       /**< name of slave */
} port_din, *pport_din;





typedef struct tag_port_dout
{
        int port_id;            /**< logical dout port_id in stack */
        int link_id;            /**< link_id in stack */
        int chan_id;            /**< chan_id in device */
        int enabled;            /**< 0 (disabled) or 1 (enabled) */
        int dev_id;             /**< id of slave */
        char dev_str[10];       /**< name of slave */
} port_dout, *pport_dout;





typedef struct tag_port_pwm
{
        int port_id;            /**< logical pwm port_id in stack */
        int link_id;            /**< link_id in stack */
        int chan_id;            /**< chan_id in device */
        int enabled;            /**< 0 (disabled) or 1 (enabled) */
        int dev_id;             /**< id of slave */
        char dev_str[10];       /**< name of slave */
} port_pwm, *pport_pwm;


typedef struct tag_par_ebox
{
        int     link_id;            /**< link_id in stack */
        int     slave_id;           /**< slave_id in stack, needed for CoE funtionality */
        char    hw_ver[6];          /**< hardware version */      
        char    sw_ver[6];          /**< software version */
        char    textline1[16];      /**< textline 1 on the display */
        char    textline2[16];      /**< textline 2 on the display */
        char    textline3[16];      /**< textline 3 on the display */
        uint8   filter_ain[2];      /**< value of summation filter of analog in */
        uint8   encoder_mode;       /**< bit 0 will enable the time stamp mode of encoder 1, encoder 2 will contain timestamp */
} par_ebox, *ppar_ebox;


int ec_EL1008_di_read_chan(double *, int, int);
int ec_EL1014_di_read_chan(double *, int, int);
int ec_EL1018_di_read_chan(double *, int, int);

int ec_EL2004_do_write_chan(double, int, int);
int ec_EL2008_do_write_chan(double, int, int);

int ec_EL3102_adc_read_chan(double *, int, int);
int ec_EL3104_adc_read_chan(double *, int, int);

int ec_EL4038_dac_write_chan(double, int, int);
int ec_EL4132_dac_write_chan(double, int, int);

int ec_EL5101_enc_read_chan(double *, int);
int ec_EL5101_latch_read_chan(double *, int);
int ec_EL5101_enc_zero_chan(int);

int ec_Ebox_adc_read_chan(double *, int, int);
int ec_Ebox_adc_read_timestamp(double *, int);
int ec_Ebox_enc_read_chan(double *, int, int);
int ec_Ebox_di_read_chan(double *, int, int);
int ec_Ebox_status_read_chan(double *, int, int);

int ec_Ebox_dac_write_chan(double, int, int);
int ec_Ebox_do_write_chan(double, int, int);
int ec_Ebox_pwm_write_chan(double, int, int);
int ec_Ebox_control_write_chan(double, int, int);
int ec_Ebox_enc_reset_chan(int, int);
int ec_Ebox_enc_zero_chan(int, int);

int ec_start(char*);
int ec_receive(void);
int ec_send(void);
int ec_io(void);
int ec_stop(void);
void ec_print_error_msg(int);

int ec_Ebox_get_ndevs(void);
int ec_get_nadc(void);
int ec_get_ndac(void);
int ec_get_nenc(void);
int ec_get_ndin(void);
int ec_get_ndout(void);
int ec_get_npwm(void);
int ec_write_ports_config_file(FILE*, int);

int ec_Ebox_get_parameters(int, ppar_ebox);
int ec_Ebox_set_parameters(int, ppar_ebox);

int ec_get_adc_port_capabilities(int, pport_adc);
int ec_get_dac_port_capabilities(int, pport_dac);
int ec_get_enc_port_capabilities(int, pport_enc);
int ec_get_din_port_capabilities(int, pport_din);
int ec_get_pwm_port_capabilities(int, pport_pwm);
int ec_adc_read_chan(double*, int);
int ec_adc_get_clipped_port(int*, int);
int ec_adc_get_state(int*, int);
int ec_adc_set_state(int, int);
int ec_dac_write_chan(double, int);
int ec_enc_read_chan(double*, int);
int ec_enc_get_state(int*, int);
int ec_enc_set_state(int, int);
int ec_din_read_chan(double*, int);
int ec_dout_write_chan(double , int);
int ec_pwm_write_chan(double , int);


