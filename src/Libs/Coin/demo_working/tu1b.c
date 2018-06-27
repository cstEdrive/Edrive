/*	tu1b.c
	======

*/

#include <stdio.h>
#include <string.h>
#include "u1.c"

char	line[20],
	fname[32] = "tu1b.u1";



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
	int	b,c,i,i1,i2,key,s;
	double	d,d1,d2;

	printf("Wait\r");
	u1ini(fn);

	i1=-10; i2=10; i=5;
	d1=-10; d2=10; d=5;
	c=1; s=0; b=1; line[0]='\0';
	do {
		key=u1key(0);
		if ( key>9 ) printf("key = %d\n",key); else
		switch ( key ) {
		case 1: { dump(); break; }
		case 2: { s=u1aib(9,s,-1,2,0);
			  i1=u1ai(1,i1); i2=u1ai(2,i2);
			  d1=u1ad(3,d1); d2=u1ad(3,d2);
			  break;
			}
		case 3: { b=u1ab(11,b); break; }
		case 4: { i=u1aib(12,i,i1,i2,s); break; }
		case 5: { d=u1adb(15,d,d1,d2,s); break; }
		case 6: { u1at(14,line,20); break; }
		case 7: { printf("Without substitutes:\n"); u1pm(99);
			  printf("With substitutes:\n");
			  u1si(1,i1); u1si(2,i2); u1sd(5,d1); u1sd(6,d2);
			  u1sb(7,b) ; u1st(8,line);
			  u1pm(99); break;
			}
		case 8: { i=u1ai(89,i); break; }
		case 9: c=0;
		}
	} while ( c );
	u1fini();
}

int	main(int argc,char **argv) {
	if ( argc<2 ) doall(fname); else doall(argv[1]);
	return 0;
}

/*	end of tu1b.c	*/
