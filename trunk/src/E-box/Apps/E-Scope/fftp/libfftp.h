/* Define window types */
#define BOXCAR      0
#define HANN        1
#define HAMMING     2

/* Define operation modes */
#define INST        0
#define AVG         1

int fftp_init(int nsignal, int N, double fs, int windowtype);
int fftp_free(int nsignal);
int fftp_update(int isignal, double *u, double *y, int mode);
int fftp_get_autopsdin(int isignal, fftw_complex *autopsdin);
int fftp_get_autopsdout(int isignal, fftw_complex *autopsdout);
int fftp_get_crosspsd(int isignal, fftw_complex *crosspsd);
int fftp_get_tfestimate(int isignal, fftw_complex *tfestimate);
int fftp_get_mscohere(int isignal, fftw_complex *mscohere);

fftw_plan fftp_init_complex_fft(int , double , fftw_complex* , fftw_complex* , double*);

int fftp_autopsd(int, fftw_complex*, fftw_complex*, double *windowscalefactor);
int fftp_crosspsd(int, fftw_complex*, fftw_complex*, fftw_complex*, double *windowscalefactor);
int fftp_tfestimate(int N, fftw_complex* tfestimate, fftw_complex* crosspsd, fftw_complex* autopsd);
int fftp_mscohere(int N, fftw_complex* mscohere, fftw_complex* crosspsd, fftw_complex* autopsdin, fftw_complex* autopsdout);

int fftp_create_window(int N, double *window, double *windowscalefactor, int type);
int fftp_apply_window(int N, double *window, fftw_complex *in);

int fftp_average_update(int N,int navg, fftw_complex* average, fftw_complex* update);
int fftp_complex_conj(int N, fftw_complex* in);
int fftsym(int N, fftw_complex *out);
int fftp_scale(int N, double *windowscalefactor, fftw_complex* in);
int fftshift(int N, fftw_complex *out, fftw_complex *out_shift);
