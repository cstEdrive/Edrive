/*TEST04: Compare data of libfftp with matlab using test04data.m
 *   
 *   Usage: Run in terminal   
 *   
 *   Inputs:  
 *   
 *   Outputs: 
 *   
 *   See also 

 *   References: 
 *   
 *   Changes: 
 *   20110923 - Initial version (M.J.C. Ronde) 
 *   20111015 - Adapted for new library version
 *   
 *   
 *   M.J.C. Ronde (2011-09-23) 
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
#include <fftw3.h>  
#include <math.h>  
 
#include "libfftp.h" 
#include "timer.h" 
  
  
#define N 1024 
  
int main(int argc, char **argv)  
{  
    FILE *fp_in, *fp_out, *fp_tfe, *fp_coh; 
    int i; 
    double t0,t1; 
      
    fftw_complex *tfe, *coherence;      
   
    double *u, *y; 
     
    /* Input and output data */ 
    u      =   malloc(sizeof(double) * N);  
    y      =   malloc(sizeof(double) * N); 
    tfe         =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N); 
    coherence   =   (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);     
     
   
    fftp_init(1, N, 2048, HANN);         
 
    /* Read input data */ 
    fp_in = fopen("test04_input.txt","r"); 
    if(fp_in == NULL){ 
        printf("Opening file failed\n"); 
        return -1;         
    } else { 
        i = 0; 
        while(fscanf(fp_in,"%lf",&u[i]) != EOF){ 
            i++; 
        }  
        fclose(fp_in); 
    }     
     
    /* Read output data */ 
    fp_out = fopen("test04_output.txt","r"); 
    if(fp_out== NULL){ 
        printf("Opening file failed\n"); 
        return -1;         
    } else { 
        i = 0; 
        while(fscanf(fp_out,"%lf",&y[i]) != EOF){ 
            i++; 
        }  
        fclose(fp_out); 
    }     
 
      
    /* Print input data */  
    for( i = 0 ; i < N ; i++ ) {  
        printf("data[%d] = { %2.2f } \t { %2.2f }\n",i, u[i], y[i] );  
    }  
     
 
 
    t0 = rtc_get_time(); 
    fftp_update(0, u, y, INST); 
    t1 = rtc_get_time(); 
    printf("TFESTIMATE+DFT took %e \n",t1-t0);     
     
    fftp_get_tfestimate(0, tfe); 
    fftp_get_mscohere(0, coherence); 
     
    fp_tfe = fopen("test04_tfe.txt","w"); 
    if(fp_tfe == NULL){ 
        printf("Opening file failed\n"); 
        return -1; 
    } else { 
        for(i = 0; i < N; i++){ 
            fprintf(fp_tfe,"%d %.15e %.15e\n",i,tfe[i][0],tfe[i][1]);  
        }  
        fclose(fp_tfe); 
    } 
     
    fp_coh = fopen("test04_coh.txt","w"); 
    if(fp_coh == NULL){ 
        printf("Opening file failed\n"); 
        return -1; 
    } else { 
        for(i = 0; i < N; i++){ 
            fprintf(fp_tfe,"%d %.15e %.15e\n",i,coherence[i][0],coherence[i][1]);  
        }  
        fclose(fp_coh); 
    }     
         
    /* Clean variables */       
    free(u);  
    free(y);  
     
    fftw_free(tfe); 
    fftw_free(coherence); 
     
    fftp_free(1); 
  
    return 0;  
}  
