/* 
 *  LIBFFTP - Library for fft processing of real data using FFTW library, 
 *            i.e. calculate transfer functions, power spectra   
 *   
 *   Usage:   
 *   
 *   Inputs:  
 *   
 *   Outputs: 
 *   
 *   See also 

 *   References: 
 *   
 *   Changes: 
 *   20110916 - Initial version (M.J.C. Ronde) 
 *   20110923 - Added autopsd/crosspsd/tfestimate/mscohere funtionality
 *   20111003 - Moved fft processing and averaging PSD inside library
 *            - Added data structure to prepare for multiple signals processing
 *   20111009 - Fixed scaling of PSD's
 *            - Added choice for instant/average mode
 *   
 *   
 *   M.J.C. Ronde (2011-09-16) 
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
#include <stdlib.h>
#include <string.h>
#include <fftw3.h> 
#include <math.h> 
 
#include "libfftp.h" 

/* Clear defenition for private functions */
#define PRIVATE static


/* TODO:
 *
 * Processing of multiple FFTs (data structure already there)
 * Add reset functionality
 *
 */

typedef struct tag_fftp_data
{
    int N;      /* Number of data point per frame */
    double fs;  /* Sample frequency [Hz]*/
    double *f;  /* Pointer for frequency vector */
    
    /* Storage of real input data in fftw format */ 
    fftw_complex *uc;
    fftw_complex *yc;
    
    /* fftw_plan */
    fftw_plan p;
    
    /* FFT of u and y */
    fftw_complex *U;
    fftw_complex *Y;
     
    /**/
    int navg;                   /* Number of averages for Cummulative Average */
    int windowtype;
    fftw_complex *autopsdin;
    fftw_complex *autopsdout;
    fftw_complex *crosspsd;   
    fftw_complex *tfestimate;
    fftw_complex *mscohere;
    fftw_complex *temp;         /* temp for calculation of the update of the auto or crosspsd */
    
    double *window;
    double *windowscalefactor;
} fftp_data, *pfftp_data;


static pfftp_data data;

int fftp_init(int nsignal, int N, double fs, int windowtype)
{
    int isignal=0;
    
    data = (pfftp_data) calloc(nsignal , sizeof(fftp_data));
    
    if (data == NULL){
        return -1;
    } else{
        for(isignal = 0; isignal < nsignal; isignal++){
            (data+isignal)->N               =   N;
            (data+isignal)->fs              =   fs;
            (data+isignal)->f               =   calloc(N, sizeof(double));
            (data+isignal)->window          =   calloc(N, sizeof(double));
            (data+isignal)->windowscalefactor =   calloc(1, sizeof(double));            
            (data+isignal)->uc              =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
            (data+isignal)->yc              =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
            (data+isignal)->U               =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
            (data+isignal)->Y               =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
            (data+isignal)->navg            =   0;
            (data+isignal)->windowtype      =   windowtype;
            (data+isignal)->autopsdin       =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
            (data+isignal)->autopsdout      =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
            (data+isignal)->crosspsd        =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
            (data+isignal)->tfestimate      =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
            (data+isignal)->mscohere        =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
            (data+isignal)->temp            =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N); /* temp for calc. updates */
            (data+isignal)->p               =   fftp_init_complex_fft(N, fs, (data+isignal)->uc, (data+isignal)->U, (data+isignal)->f);
            fftp_create_window(N, (data+isignal)->window, (data+isignal)->windowscalefactor, windowtype);
        }
        return 0;
    }
}
int fftp_free(int nsignal)
{
    int isignal=0;
    
    printf("Cleaning up \n");
    for(isignal = 0; isignal < nsignal; isignal++){
        free((data+isignal)->f);
        free((data+isignal)->window);
        free((data+isignal)->windowscalefactor);        
        
        /**/
        fftw_free((data+isignal)->uc);
        fftw_free((data+isignal)->yc);
        fftw_free((data+isignal)->U);
        fftw_free((data+isignal)->Y);
        fftw_free((data+isignal)->autopsdin);
        fftw_free((data+isignal)->autopsdout);
        fftw_free((data+isignal)->crosspsd);
        fftw_free((data+isignal)->tfestimate);
        fftw_free((data+isignal)->mscohere);
        fftw_free((data+isignal)->temp);
        
        fftw_destroy_plan((data+isignal)->p);
    }
    
    fftw_cleanup();
    
    free(data);
    
    return 0;  
}

int fftp_update(int isignal, double *u, double *y, int mode)
{
    /* TODO
     */
    int i,N;
    int nsignals=0; /* Number of signals to process */
    
    N = (data+isignal)->N;

      
    if(u != NULL){
        nsignals++;
        for(i = 0; i < N; i++){
            ((data+isignal)->uc)[i][0] = u[i];
            ((data+isignal)->uc)[i][1] = 0.0;            
        }
        fftp_apply_window((data+isignal)->N, (data+isignal)->window, (data+isignal)->uc);
        fftw_execute_dft((data+isignal)->p, (data+isignal)->uc, (data+isignal)->U);   
    } else {
        printf("u not defined \n");
    }
    
    if(y != NULL){
        nsignals += 2;
        for(i = 0; i < N; i++){
            ((data+isignal)->yc)[i][0] = y[i];
            ((data+isignal)->yc)[i][1] = 0.0;            
        }
        fftp_apply_window((data+isignal)->N, (data+isignal)->window, (data+isignal)->yc);        
        fftw_execute_dft((data+isignal)->p, (data+isignal)->yc, (data+isignal)->Y);     
    } else {
        printf("y not defined \n");
    }    
    
    switch(mode){
        case AVG:
            printf("mode=AVG\n");
            switch(nsignals){
                case 0:
                    printf("No signals to process\n");
                    return -1;
                case 1:
                    printf("Autopsd calculation u only \n");
                    fftp_autopsd((data+isignal)->N, (data+isignal)->temp, (data+isignal)->U, (data+isignal)->windowscalefactor);
                    fftp_average_update((data+isignal)->N, (data+isignal)->navg, (data+isignal)->autopsdin, (data+isignal)->temp);
                    
                    /* Correct place to update navg ?*/
                    (data+isignal)->navg++;
                    printf("navg = %d \n",(data+isignal)->navg);
                    return 0;
                case 2:
                    printf("Autopsd calculation y only \n");
                    fftp_autopsd((data+isignal)->N, (data+isignal)->temp, (data+isignal)->Y, (data+isignal)->windowscalefactor);
                    fftp_average_update((data+isignal)->N, (data+isignal)->navg, (data+isignal)->autopsdout, (data+isignal)->temp);
                    
                    /* Correct place to update navg ?*/
                    (data+isignal)->navg++;
                    printf("navg = %d \n",(data+isignal)->navg);
                    return 0;
                case 3:
                    printf("Full calculation, navg=%d \n",(data+isignal)->navg);
                    fftp_autopsd((data+isignal)->N, (data+isignal)->temp, (data+isignal)->U, (data+isignal)->windowscalefactor);
                    fftp_average_update((data+isignal)->N, (data+isignal)->navg, (data+isignal)->autopsdin, (data+isignal)->temp);
                    
                    fftp_autopsd((data+isignal)->N, (data+isignal)->temp, (data+isignal)->Y, (data+isignal)->windowscalefactor);
                    fftp_average_update((data+isignal)->N, (data+isignal)->navg, (data+isignal)->autopsdout, (data+isignal)->temp);
                    
                    fftp_crosspsd((data+isignal)->N, (data+isignal)->temp, (data+isignal)->Y, (data+isignal)->U, (data+isignal)->windowscalefactor);
                    fftp_average_update((data+isignal)->N, (data+isignal)->navg, (data+isignal)->crosspsd, (data+isignal)->temp);
                    
                    fftp_tfestimate((data+isignal)->N, (data+isignal)->tfestimate, (data+isignal)->crosspsd, (data+isignal)->autopsdin);
                    fftp_mscohere((data+isignal)->N, (data+isignal)->mscohere, (data+isignal)->crosspsd, (data+isignal)->autopsdin, (data+isignal)->autopsdout);
                    
                    /* Correct place to update navg ?*/
                    (data+isignal)->navg++;
                    printf("navg = %d \n",(data+isignal)->navg);
                    return 0;
                    default:
                        printf("Error\n");
                        return -1;
            } /* End switch nsignals (mode=AVG)*/
        case INST:
            printf("mode=INST\n");
            switch(nsignals){
                case 0:
                    printf("No signals to process\n");
                    return -1;
                case 1:
                    printf("Autopsd calculation u only \n");
                    fftp_autopsd((data+isignal)->N, (data+isignal)->autopsdin, (data+isignal)->U, (data+isignal)->windowscalefactor);
                    return 0;
                case 2:
                    printf("Autopsd calculation y only \n");
                    fftp_autopsd((data+isignal)->N, (data+isignal)->autopsdout, (data+isignal)->Y, (data+isignal)->windowscalefactor);
                    return 0;
                case 3:
                    printf("Full calculation, navg=%d \n",(data+isignal)->navg);
                    fftp_autopsd((data+isignal)->N, (data+isignal)->autopsdin, (data+isignal)->U, (data+isignal)->windowscalefactor);
                    
                    fftp_autopsd((data+isignal)->N, (data+isignal)->autopsdout, (data+isignal)->Y, (data+isignal)->windowscalefactor);
                    
                    fftp_crosspsd((data+isignal)->N, (data+isignal)->crosspsd, (data+isignal)->Y, (data+isignal)->U, (data+isignal)->windowscalefactor);                  
                    
                    fftp_tfestimate((data+isignal)->N, (data+isignal)->tfestimate, (data+isignal)->crosspsd, (data+isignal)->autopsdin);
                    fftp_mscohere((data+isignal)->N, (data+isignal)->mscohere, (data+isignal)->crosspsd, (data+isignal)->autopsdin, (data+isignal)->autopsdout);
                    
            /* Correct place to update navg ?*/
                    (data+isignal)->navg++;
                    printf("navg = %d \n",(data+isignal)->navg);
                    return 0;
                    default:
                        printf("Error\n");
                        return -1;
            } /* End switch nsignals (mode=INST)*/
        default:
            printf("Error: unknown mode\n");
            return -1;
    } /* End switch mode*/
            
}

int fftp_get_autopsdin(int isignal, fftw_complex *autopsdin)
{
    memcpy(autopsdin,(data+isignal)->autopsdin,(data+isignal)->N*sizeof(fftw_complex));
    
    return 0;
}

int fftp_get_autopsdout(int isignal, fftw_complex *autopsdout)
{
    memcpy(autopsdout,(data+isignal)->autopsdin,(data+isignal)->N*sizeof(fftw_complex));
    
    return 0;
}

int fftp_get_crosspsd(int isignal, fftw_complex *crosspsd)
{
    memcpy(crosspsd,(data+isignal)->crosspsd,(data+isignal)->N*sizeof(fftw_complex));
    
    return 0;
}

int fftp_get_tfestimate(int isignal, fftw_complex *tfestimate)
{
    memcpy(tfestimate,(data+isignal)->tfestimate,(data+isignal)->N*sizeof(fftw_complex));
    
    return 0;
}

int fftp_get_mscohere(int isignal, fftw_complex *mscohere)
{
    memcpy(mscohere,(data+isignal)->mscohere,(data+isignal)->N*sizeof(fftw_complex));
    
    return 0;
}
 
fftw_plan fftp_init_complex_fft(int N, double fs, fftw_complex *in, fftw_complex *out, double *f) 
{
    /* Create data structures and plan for N-point fft */
    fftw_plan p;
    int i=0,k;
    double f0;
           
    
    /* Ground frequency */
    f0 = fs/(double) N;
        
    if ((N % 2)==0){
        /* N is even 
         * k=-N/2:N/2-1; 
         */
        for(k=-(N/2) ; k < (N/2); k++){
            f[i] = k*f0;
            i++;    /* Index frequency vector */
        } 
    } else {
        /* N is odd
         * k=-(N-1)/2:(N-1)/2; 
         */
        printf("Not supported\n");
    }
     
    /* Create plan */
    p       =   fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 
    fftw_print_plan(p);    
    printf("\n");
    
    return p;
}

#if 0
fftw_plan fftp_init_real_fft(int N, double fs, double *in, fftw_complex *out, double *f)  
{
    /* Create data structures and plan for N-point fft */
    fftw_plan p;
    
    /* Create plan */
    p       =   fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE); 
    
    return p;
}
#endif

int fftp_average_update(int N,int navg, fftw_complex* average, fftw_complex* update)
{
    int i;
    
    if(N <= 0){
        return -1;
    }
    
    /* Cummulative moving average of complex number 
     * CA(n+1) = (n*CA(n) + xnew) / (n+1)
     */
    
    for(i=0; i<N; i++){
        average[i][0]= (navg*average[i][0]+update[i][0])/(navg+1); /* Real part     */
        average[i][1]= (navg*average[i][1]+update[i][1])/(navg+1); /* Complex part  */
        
    }
    
    return 0;
}

int fftp_complex_conj(int N, fftw_complex* in)
{
    int i;
    
    for( i = 0 ; i < N ; i++ ) { 
        in[i][1] = -in[i][1];
    }
    
    return 0;
}
 
int fftp_autopsd(int N, fftw_complex* autopsd, fftw_complex* in, double *windowscalefactor)
{
    /* Auto PSD, i.e. WY*, where * denotes the complex conjugate of the Fourier coefficients */
     
    int i;
    
    for( i = 0 ; i < N ; i++ ) { 
        /* divide by N for scaling */
        autopsd[i][0] = (pow(in[i][0],2)+pow(in[i][1],2));
        autopsd[i][1] = 0.0;
    }
    
    fftp_scale(N, windowscalefactor, autopsd);
    
    return 0;
}

int fftp_crosspsd(int N, fftw_complex* crosspsd, fftw_complex* in1, fftw_complex* in2, double *windowscalefactor)
{
     /* Cross PSD, i.e. WY*, where * denotes the complex conjugate of the Fourier coefficients
      * W = (a+bi) and Y = (c+di)
      * then
      * WY* = ac+bd + (bc-ad)*i
      */
    int i;

    for( i = 0 ; i < N ; i++ ) { 
        crosspsd[i][0] = in1[i][0]*in2[i][0]+in1[i][1]*in2[i][1];
        crosspsd[i][1] = -in1[i][0]*in2[i][1]+in1[i][1]*in2[i][0];
    }
    
    fftp_scale(N, windowscalefactor, crosspsd);
    
    return 0;
}

int fftp_tfestimate(int N, fftw_complex* tfestimate, fftw_complex* crosspsd, fftw_complex* autopsd)
{
    /* tfestimate, i.e. divide the crosspsd by autopsd */
    int i;
    double temp;
     
    /* Estimate transfer function using (averaged) power spectra */
    for( i = 0 ; i < N ; i++){
        temp =  pow(autopsd[i][0],2)+pow(autopsd[i][1],2);
        tfestimate[i][0] = (crosspsd[i][0]*autopsd[i][0]+crosspsd[i][1]*autopsd[i][1])/temp;
        tfestimate[i][1] = (-crosspsd[i][0]*autopsd[i][1]+crosspsd[i][1]*autopsd[i][0])/temp;
    }
        
    return 0;
}

int fftp_mscohere(int N, fftw_complex* mscohere, fftw_complex* crosspsd, fftw_complex* autopsdin, fftw_complex* autopsdout)
{
    /* mscohere */
    int i;    
    
    /* Compute coherence funtion
     * 1. Compute |Pxy|^2=|a+bi|^2=a^2+b^2
     * 2. Compute PxxPyy, note that Im(Pxx)=Im(Pyy) = 0.0
     */
    for( i = 0 ; i < N ; i++){    
        mscohere[i][0] = (pow(crosspsd[i][0],2) + pow(crosspsd[i][1],2))/(autopsdin[i][0]*autopsdout[i][0]);
        mscohere[i][1] = 0.0;
    }
       
    return 0;    
}

int fftp_create_window(int N, double *window, double *windowscalefactor, int type)
{
    int i;
    
    switch(type){
        case BOXCAR:
            for( i = 0 ; i < N ; i++){
                window[i] = 1.0;
                windowscalefactor[0] = windowscalefactor[0]+pow(window[i],2);
            }                        
            break;
        case HANN:
            for( i = 0 ; i < N ; i++){
                window[i] = 0.5 * (1 - cos(2*M_PI*i/(N-1)));
                windowscalefactor[0] = windowscalefactor[0]+pow(window[i],2);                
            }                        
            break;
        case HAMMING:
            for( i = 0 ; i < N ; i++){
                window[i] = 0.54 - 0.46 * cos(2*M_PI*i/(N-1));
                windowscalefactor[0] = windowscalefactor[0]+pow(window[i],2);                
            }                        
            break;            
        default:
            printf("Undefined window\n");
            return -1;
    }
    
    windowscalefactor[0] = windowscalefactor[0] / N;
    return 0;
}

int fftp_apply_window(int N, double *window, fftw_complex *in)
{
    int i;
    
    for( i = 0 ; i < N ; i++){
        in[i][0]=in[i][0]*window[i];
        in[i][1]=in[i][1]*window[i];
    }

    return 0;
}


int fftp_scale(int N, double *windowscalefactor, fftw_complex* in) 
{ 
    int i,temp; 
     
    /* Normalize the fft */ 
    for(i = 0; i < N; i++){
        in[i][0] = (in[i][0]/(double) pow(N,2));
        in[i][1] = (in[i][1]/(double) pow(N,2));
    } 
    
    for(i = 0; i < N; i++){
        in[i][0] = (in[i][0])/windowscalefactor[0];
        in[i][1] = (in[i][1])/windowscalefactor[0];
    }
    /* Scale everything by 2, except for DC and Nyquist*/
    temp = (int) ceil((double) N/2);     
    for(i = 1; i < temp; i++){
        in[i][0] = (2*in[i][0]);
        in[i][1] = (2*in[i][1]);
    }
    return 0; 
} 

int fftsym(int N, fftw_complex* out) 
{ 
    int i; 
     
    /* Symmetry for DFTs of real data */ 
    for(i = (N/2+1); i < N; i++){ 
        out[i][0] =  out[N-i][0]; 
        out[i][1] = -out[N-i][1]; 
    } 
     
    return 0; 
} 
     
     
     
int fftshift(int N, fftw_complex* out, fftw_complex* out_shift) 
{ 
    /* Generate shifted version, i.e. move DC to center of the spectrum */ 
    int i,n2; 
     
    n2 = (int) ceil((double) N/2); 
    for( i = 0 ; i < N ; i++ ) { 
        if(i < n2){ 
            out_shift[i][0] = out[n2+i][0]; 
            out_shift[i][1] = out[n2+i][1]; 
        } else { 
            out_shift[i][0] = out[i-n2][0]; 
            out_shift[i][1] = out[i-n2][1]; 
        } 
    } 
     
    return 0; 
} 
