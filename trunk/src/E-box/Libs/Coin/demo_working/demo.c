/*	demo.c
	======

	Demonstration program for COIN.
	Does some very simple random-generator testing and
	shows how to use COIN in an interactive pogram.
	Uses build-in function random.
*/

#define	mteach	 	  11
#define	maxv	      	1000
#define	maxh		  20
#define	maxt		 100

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "u1.h"


double	v[maxv];		/* List of random values      	*/
int	h[maxh];		/* Histogram values		*/
char	title[maxt] = "Demo";	/* Title                       	*/
int	nh,                     /* Number of histogram intervals*/
	ns,                     /* Number of values to be skipped*/
	nv,			/* Number of random values	*/
	gd;			/* Generate Done		*/


void	gen(void) {
/*	Generates random values v[0]..v[nv-1] by using the build-in
	function random and skipping ns values after every accepted
	value
*/
	int i,j;

	for ( i=0; i<nv; i++ ) {
		v[i]=(double)rand()/RAND_MAX;
		for  ( j=0; j<ns; j++ ) rand();
	}
	gd=1;
}


void	analys(void) {
/*	Computes and reports mean and standard deviation of v[0]..v[nv-1]
*/
	int i; double a,dev,m,s;

	if ( gd==0 ) gen();
	s=0; for ( i=0; i<nv; i++ ) s+=v[i];
	m=s/nv;
	s=0; for ( i=0; i<nv; i++ ) { a=v[i]-m; s+=a*a; }
	dev=sqrt(s/(nv-1));
	u1si(2,nv); u1sd(3,m); u1sd(4,dev); u1pm(12);
}

void	hist(void) {
/*	Makes a simple histogram (with nh intervals) of v[0]..v[nv-1]
*/
	int i; double d;

	if ( gd==0 ) gen();
	d=1.0/nh;
	for ( i=0; i<nh; i++ ) h[i]=0;
	for ( i=0; i<nv; i++ ) h[(int)(v[i]/d)]++;
	u1st(1,title); u1si(2,nv); u1pm(13);
	for ( i=0; i<nh; i++ )
		printf("%5.3lf - %5.3lf : %5d\n",i*d,(i+1)*d,h[i]);
}

void	doall(void) {
	int key,teach;

	nv=100; nh=5; ns=0; gd=0;

	u1ini("demo.u1");	/* Initialize 				*/
	teach=1;
	u1hlpm(20+teach);	/* Lessons are messages 21, 22 etc.	*/
	do {
		key=u1key(0);	/* Fetch command 			*/
		switch ( key ) {
		case 1:	{ u1at(1,title,maxt); break; }
		case 2: { nv=u1aib(2,nv,10,maxv,0);
			  ns=u1aib(3,ns,0,7,0);
			  gd=0; break;
			}
		case 3: { nh=u1aib(4,nh,1,maxh,0); break; }
		case 4: { u1st(1,title);
			  u1si(2,nv); u1si(3,ns); u1si(4,nh);
			  u1pm(11); break;
			}
		case 5: { gen(); break; }
		case 6: { analys(); break; }
		case 7: { hist(); break; }
		case 11:{ if ( teach<mteach ) teach++;
			  u1hlpm(20+teach); break;
			}
		case 12:{ teach=u1aib(20,teach,1,mteach,0);
			  u1hlpm(20+teach); break;
			}
		case 10:break;
		case 99:break;
		}
	} while ( key!=99 );
	u1fini();
}


int	main(void) {
	doall();
	return 0;
}

/*	end of demo.c	*/
