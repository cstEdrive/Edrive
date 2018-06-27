/*	tu1c.c
	======

*/

#include <stdio.h>
#include "u1.h"

char	fname[32] = "tu1c.u1";
extern	int	*u1k;
extern	char	*u1t;


void	dump(void) {
	char	*tp;
	int	j,j1,j2,nk,nt;

	u1memc(&nk,&nt);
	printf("u1k : n=%d",nk);
	for ( j=0; j<nk; j++ ) {
		if ( j%12==0 ) printf("\n%4d:",j);
		else if ( j%6==0 ) printf("      ");
		printf("%5d",u1k[j]);
	}
	printf("\nu1t : n=%d",nt);
	tp=u1t;
	for ( j=0; j<nt; j++ ) {
		if ( j%30==0 ) printf("\n%4d:",j);
		else if ( j%10==0 ) printf("   ");
		j1=32; j2=(int)*tp++;
		if ( j2<32 ) { j1=94; j2+=64; }
		printf("%c%c",(char)j1,(char)j2);
	}
	printf("\n");
}

void	doall(char *fn) {
	int	key;

	printf("Wait\r");
	u1ini(fn);

	do { 	key=u1key(0);
		u1si(1,key); u1pm(1);
		if ( key==8 ) dump();
	} while ( key!=9 );

	u1fini();
}

int	main(int argc,char **argv) {
	if ( argc<2 ) doall(fname); else doall(argv[1]);
	return 0;
}

/*	end of tu1c.c	*/
