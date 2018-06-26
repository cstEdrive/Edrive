
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "u1.h"

int cmd_go(void)
{
    printf("Hallo, go!\n");
    
    return 0;
}




int cmd_halt(void)
{
    printf("Halt!!!\n");

    return 0;
}




int cmd_time(void)
{
    printf("What's the time, dude?!\n");

    return 0;
}





int domenu(void)
{
    int key;
    
    u1ini("wintarget.u1");

    do {
	key=u1key(0);
	switch ( key ) {
	case 10:
	    cmd_go();
	    break;
	case 20:
	    cmd_halt();
	    break;
	case 30:
	    cmd_time();
	    break;
	case 99:
	    break;
	}
    } while ( key!=99 );

    u1fini();
}





int main(void)
{
	domenu();
	return 0;
}
