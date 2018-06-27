/*TEST_ROOT   
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
 *   20120719 - Initial version (E. Cerit) 
 *   20120719 - Added header
 *   
 *   M.J.C. Ronde (2012-07-19) 
 *   ________________________________
 *   Eindhoven University of Technology
 *   Dept. of Mechanical Engineering
 *   Control Systems Technology group
 *   PO Box 513, GEM-Z -1.127
 *   5600 MB Eindhoven, NL
 *   T +31 (0)40 247 2798
 *   F +31 (0)40 246 1418
 *   E m.j.c.ronde@tue.nl
 */ 
 
#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
 
  int main(void) 
  { 
     uid_t euid; 
 
     euid = geteuid(); 
      
     printf("The effective user id is  %d. \n\n", (int) geteuid()); 
      
     if (euid == 0) { 
        printf("You are ROOT. \n"); 
     } 
      
     if (euid != 0) { 
        printf("ATTENTION!\nYou do not have the ROOT permission!\n"); 
        printf("Type '>sudo su' to change your user profile. \n"); 
     } 
     return 0;              
  } 
 
 
/* uid_t = This is an integer data type used to represent user IDs.  
 * In the GNU library, this is an alias for unsigned int*/ 
 
/* getuid(void) = The getuid function returns the real user ID of the  
 * process.*/ 
 
/* geteuid(void) = The geteuid function returns the effective user ID of  
 * the process.*/ 
