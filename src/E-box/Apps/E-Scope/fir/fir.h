
// macros
#define FIRNTAPS	101
#define FIRNBANDS	2
#define BANDPASS	1
#define MAXFIR		10

/* prototypes */
int fir_init(int);
int fir_design(double, int);
double fir_filter(double, int);
