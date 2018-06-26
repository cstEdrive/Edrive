
/* macros */
#define SINE		1
#define RAND		2
#define SQUARE		3
#define TWOTONE		4
#define CHIRP		5
#define MULTISINE	6
#define WHISTLE		7
#define PI		3.14159265358979

/* prototypes */
double fg_get_val(double, int);
int fg_start(int);
int fg_stop(int);
int fg_status(int);
int fg_set_defaults(void);
int fg_set_amp1(double, int);
int fg_get_amp1(double*, int);
int fg_set_amp2(double, int);
int fg_get_amp2(double*, int);
int fg_set_freq1(double, int);
int fg_get_freq1(double*, int);
int fg_set_freq2(double, int);
int fg_get_freq2(double*, int);
int fg_set_offset(double, int);
int fg_get_offset(double*, int);
int fg_set_type(int, int);
int fg_get_type(double*, int);
int fg_set_onoff(int);
int fg_get_onoff(double*);
int fg_notify(int);
int fg_set_hold_order(int, int);
int fg_get_hold_order(double*, int);
int fg_put_maxfreq(double);
int fg_put_obsiv(double);
int fg_put_issim(int);
