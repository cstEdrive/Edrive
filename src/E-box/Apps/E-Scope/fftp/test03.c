#include <stdio.h> 
#include <stdlib.h>
#include <fftw3.h> 
#include <math.h> 

#include "libfftp.h"
#include "timer.h"
 
 
#define N 1024
 
int main(int argc, char **argv) 
{ 
    FILE *fp_in, *fp_out;
    int i;
    double temp,t0,t1;
     
    fftw_complex *in, *out;   
    fftw_plan p;    
  
    double *f;
    
    /* Input and output data */
    in      =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N); 
    out     =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
    /* Frequency vector (real) */
    f       =   malloc(sizeof(double) *N);
   
    p = fftp_init_complex_fft(N, 2048, in, out, f);
    printf("\n\n");
    fftw_print_plan(p);
    
    for(i = 0; i < N; ++i){ 
        printf("%d - %.15e\n",i,f[i]);
    }
    
    /* Create input data */
    for(i = 0; i < N; ++i){ 
        temp = (double) i/ (double) N; 
        in[i][0] = sin(temp*2*M_PI*5); 
        in[i][1] = 0.0; 
        out[i][0] = 0.0; 
        out[i][1] = 0.0; 
    } 
     
    /* Print input data */ 
    for( i = 0 ; i < N ; i++ ) { 
        //printf("data[%d] = { %2.2f, %2.2f }\n",i, in[i][0], in[i][1] ); 
    } 
    
    fp_in = fopen("input_data.txt","w"); 
    if(fp_in == NULL){
        printf("Opening file failed\n");
        return -1;        
    } else {
         /* Write to file*/
        for( i = 0 ; i < N ; i++ ) {
            temp = (double) i/ (double) N;
            fprintf(fp_in,"%.15e\n",sin(temp*2*M_PI*5)); 
        } 
        fclose(fp_in);
    }
    
    return 0;
    
    t0 = rtc_get_time();
    fftw_execute(p); 
    t1 = rtc_get_time();
    printf("DFT took %e \n",t1-t0);
     

    /* Print output data */
    for(i = 0; i < N; i++){ 
       //printf("%d %e %ei\n",i,out[i][0],out[i][1]); 
    }
    
    
    fp_out = fopen("output_data.txt","w");
    if(fp_out == NULL){
        printf("Opening file failed\n");
        return -1;
    } else {
        for(i = 0; i < N; i++){
            fprintf(fp_out,"%d %.15e %.15e\n",i,out[i][0],out[i][1]); 
        } 
        fclose(fp_out);
    }
    
    
    /* Clean variables */ 
    fftw_destroy_plan(p); 
  
    fftw_free(in); 
    fftw_free(out); 
    
    free(f);
 
    return 0; 
} 
