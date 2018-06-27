/* TEST06: Compare data of libfftp with matlab using test06data.m
 *   
 *   Usage:  Run in terminal      
 *   
 *   Inputs:  
 *   
 *   Outputs: 
 *   
 *   See also 

 *   References: 
 *   
 *   Changes: 
 *   20111003 - Initial version (M.J.C. Ronde) 
 *   20111015 - Added header
 *   
 *   
 *   M.J.C. Ronde (2011-10-03) 
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
    FILE *fp_in, *fp_out, *fp_tfe, *fp_coh, *fp_psdu; 
    double *buffer_input,*buffer_output, *u, *y; 
    int i,iframe=0,nframes; 
     
    fftw_complex *tfe,*coh,*psdu; 
     
    buffer_input = calloc(N*10,sizeof(double)); 
    buffer_output = calloc(N*10,sizeof(double)); 
    u = calloc(N,sizeof(double)); 
    y = calloc(N,sizeof(double)); 
     
    tfe =     (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N); 
    coh =     (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);     
    psdu=     (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);      
     
    /* Read input data */ 
    fp_in = fopen("test06_input.txt","r"); 
    if(fp_in == NULL){ 
        printf("Opening file failed\n"); 
        return -1;         
    } else { 
        i = 0; 
        while(fscanf(fp_in,"%lf",&buffer_input[i]) != EOF){ 
            i++; 
        }  
        fclose(fp_in); 
    }    
    printf("%d lines read \n",i);     
     
     /* Read output data */ 
    fp_out = fopen("test06_output.txt","r"); 
    if(fp_out== NULL){ 
        printf("Opening file failed\n"); 
        return -1;         
    } else { 
        i = 0; 
        while(fscanf(fp_out,"%lf",&buffer_output[i]) != EOF){ 
            i++; 
        }  
        fclose(fp_out); 
    }     
    printf("%d lines read \n",i); 
    nframes = (i/N); 
     
     
    fftp_init(2, N, 1024.0,HANN); 
     
    for(iframe = 0; iframe < nframes; iframe++){ 
        /* Fill u and y with fresh data from buffer */ 
        for(i = 0; i < N; i++){ 
            u[i] = buffer_input[i+iframe*N]; 
            y[i] = buffer_output[i+iframe*N]; 
        } 
         
        fftp_update(0, u, y, AVG); 
    } 
     
    fftp_get_tfestimate(0, tfe); 
    fftp_get_mscohere(0, coh);     
    fftp_get_autopsdin(0, psdu); 
     
     
    fp_tfe = fopen("test06_tfe.txt","w"); 
    if(fp_tfe == NULL){ 
        printf("Opening file failed\n"); 
        return -1; 
    } else { 
        for(i = 0; i < N; i++){ 
            fprintf(fp_tfe,"%d %.15e %.15e\n",i,tfe[i][0],tfe[i][1]);  
        }  
        fclose(fp_tfe); 
    } 
     
    fp_coh = fopen("test06_coh.txt","w"); 
    if(fp_coh == NULL){ 
        printf("Opening file failed\n"); 
        return -1; 
    } else { 
        for(i = 0; i < N; i++){ 
            fprintf(fp_coh,"%d %.15e %.15e\n",i,coh[i][0],coh[i][1]);  
        }  
        fclose(fp_coh); 
    } 
     
    fp_psdu = fopen("test06_psdu.txt","w"); 
    if(fp_psdu == NULL){ 
        printf("Opening file failed\n"); 
        return -1; 
    } else { 
        for(i = 0; i < N; i++){ 
            fprintf(fp_psdu,"%d %.15e %.15e\n",i,psdu[i][0],psdu[i][1]);  
        }  
        fclose(fp_psdu); 
    } 
     
    /* Clean up library */ 
    fftp_free(2); 
     
    free(buffer_input); 
    free(buffer_output); 
    free(u); 
    free(y); 
     
    fftw_free(tfe); 
    fftw_free(coh);  
    fftw_free(psdu); 
     
     
    return 0; 
} 
