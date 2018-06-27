
#include <math.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    int i;
    double t;
    double u[8],y[4];
    
    td_init();
    
    tde_start_acq_simulink(100.0);
    
    for (i=0;i<100000;i++) {
	t=((double) i)/1000.0;
	u[0]=sin(2*3.14*10.0*t);
	tde_timer_fun_simulink(u,y);
	printf("t=%f   u=%f   y=%f\n",t,u[0],y[0]);
	usleep(10000);
    }

    return 0;
}
