/*	u1.c
	====

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "u1.h"

/*
	u1imax	- size of user input buffer
	u1omax	- size of user output buffer
	u1kmax	- size of u1k-area
	u1tmax	- size of u1t-area
		  See description below.
*/
#define	u1imax    126
#define	u1omax	  500
int	u1kmax = 2000;
int	u1tmax = 8000;

void	u1em(int i,char *s) {
	/* Defines one of four error messages for i=1..4. Message is
	   written in s. See below for intended content of the messages.
	*/
	if ( i==1 ) strcpy(s,"--more(~1)--");
	if ( i==2 ) strcpy(s,"sorry, no help");
	if ( i==3 ) strcpy(s,"interval is ~1 .. ~2");
	if ( i==4 ) strcpy(s,"fatal error ~1 (~2)");
}


/**********************************************************************	*/
/* Keep everything unchanged below this 				*/

#define u1smax     32

FILE    *u1fi = NULL,       /* user-redirected input 	*/
	*u1fo = NULL,       /* user-redirected output	*/
	*u1fc = NULL;       /* command file  		*/

	/* For meaning, see description below	*/
int     *u1k    = NULL;
char    *u1t    = NULL;
long	*u1h    = NULL;
char    u1cd[4] = { '?',' ','Y','N' };
int     u1type[128];
char	*u1tp,*u1tq,*u1tr;
int	u1hp;
int     u1s[u1smax],u1sp;
char    u1i[u1imax+1],*u1ip,*u1ir;
char	u1j[u1imax+1],*u1jp;
char    u1o[u1omax+1],*u1op,*u1oq;
int     u1sbl[11],u1scl[11],u1sil[11];
double  u1sdl[11];
char	*u1stl[11];
int     u1l;


/* --- Description of system independent global variables ------------- */
/*

	u1h   : file pointers to help texts
	u1hp  : last position used in u1h

	u1i   : user input buffer
	u1ip  : current position in u1i
	u1ir  : previous position in u1i

	u1j   : copy of u1i
	u1jp  : copy of u1ip

	u1k   : storage area for integers.
		Filled by u1ini. Contains:
		u1k[0]   : number of u1k-elements in use
		u1k[1]   : pointer to first command
		u1k[2]   : number of integers to represent a command (6)
		u1k[3]   : pointer to first message
		u1k[4]   : number of integers to represent a message (3)
		u1k[5]   : number of u1k-elements in use

		Above this for every command:
		u1k[p  ] : key
		u1k[p+1] : reference to help text:
			   >0 : index in u1h
			   =0 : no help is available
			   -1 : don't know yet if help is available.
		u1k[p+2] : pointer to text of command (no substitutions)
		u1k[p+3] : pointer to next command of same level (0 when
			   not present)
		u1k[p+4] : pointer to first subcommand (0 when not
			   present)
		u1k[p+5] : 64*a+32*c+n
			   where: n = abbreviation length (1..31)
				  c = type of abbreviation (0 with +,
				      1 with - in control file)
				  a = action (0 with space, 1 with -,
				      2 with !, 3 with / in control file)

		Above this for every message:
		u1k[p  ] : message number
		u1k[p+1] : reference to help text, see above
		u1k[p+2] : pointer to text of message

	u1l   : line number from command file (used in u1ini error
		messages only)

	u1o   : user output buffer
	u1op  : last position filled in u1o

	u1s   : stack of command-pointers
	u1sp  : top-of-stack position in u1s

	u1scl : substitute control list, indexed by substitution number
		(10 is used for prompting during ask-actions):
		0 = no substitution
		1 = boolean (value in corresponding position of u1sbl)
		2 = integer ( ..           ..          ..       u1sil)
		3 = double  ( ..           ..          ..       u1sdl)
		4 = text    (value in corresponding position of u1stl
			     points to string in u1t)
	u1sbl : boolean values to substitute
	u1sil : integer values to substitute
	u1sdl : double  values to substitute
	u1stl : string  values to substitute (pointers to strings in u1t)

	u1t   : storage area for characters.
		Contains strings closed by '\0'.
		Filled with fixed information by u1ini and -above that-
		with variable information by text-substitution routines.
		Special character values are:
		0    : string terminator
		1..9 : substitute number
		10   : new line
		11   : bell
	u1tp  : first free position in u1t after fixed information
	u1tq  : last available position in u1t
	u1tr  : first free position in u1t after variable information
		(i.e. fixed information and text-substitutions)

	u1type: type of ascii number : -1 = '\0'  (i.e. end-of-string)
					0 = space
					1 = digit
					2 = A..Z, a..z and _ (underscore)
					3 = other
*/


/* --- Miscellaneous internal functions ------------------------------- */

#define u1back()        { u1ip=u1ir; }
	/* Backs up to last marked position in u1i */

void	u1cs(void) { int i;
	/* Clears substitute information */
	u1tr=u1tp; for ( i=1; i<11; i++ ) { u1scl[i]=0; u1stl[i]=u1t; }
}

void	u1clr(void) { u1ip=u1ir=u1i; *u1ip='\0'; u1op=u1o; u1cs(); }
	/* Clears u1i, u1o and substitutions */

char	u1skp(int n) {
	/* Skips n characters and trailing spaces from u1i.
	   Returns current character.
	*/
	u1ip+=n; while ( *u1ip==' ' ) u1ip++;
	u1ir=u1ip;
	return *u1ip;
}

void	u1stop(void) { exit(1); }

char	u1upc(char c) { return ( (c>96 && c<123) ? c-32 : c ); }


/* --- Put functions -------------------------------------------------- */

void	u1pl(FILE *f,char *s,int eol) {
	/* Writes string s to file f. With eol != 0, an end-of-line is
	   appended. With eol < 0 , output may be delayed (buffered).
	*/
	fputs(s,f); if ( eol ) fprintf(f,"\n");
}

void	u1put(int eol) {
	/* Outputs the content of u1o */
	*u1op='\0'; u1pl(stdout,u1o,eol); u1op=u1o;
}

void	u1pc(char c)	{ if ( u1op<u1oq ) *u1op++=c; }
	/* Puts a character into u1o */

void	u1pt(char *t) { while ( *t!='\0' ) u1pc(*t++); }
	/* Puts text t in u1o */

void	u1pb(int b) { u1pc( b ? u1cd[2] : u1cd[3] ); }
	/* Puts a boolean value in u1o */

void	u1pi(int i) { char s[20],*d;
	/* Puts an integer value in u1o */
	if ( i<0 ) u1pc('-');
	i=abs(i); d=s;
	do { *d++=i%10+'0'; i/=10; } while ( i!=0 );
	while ( d>s ) u1pc(*(--d));
}

void	u1pd(double d) { char s[20];
	/* Puts a double precision value in u1o */
	sprintf(s,"%1.5g",d); u1pt(s);
}

void	u1ps(int s) {
	/* Puts substitute i to u1o */
	switch ( u1scl[s] ) {
	case 1: { u1pb(u1sbl[s]); break; }
	case 2: { u1pi(u1sil[s]); break; }
	case 3: { u1pd(u1sdl[s]); break; }
	case 4: { u1pt(u1stl[s]); break; }
	}
}

void	u1pu(void) { u1pl(stdout,u1i,-1); u1op=u1o; }
	/* Copies content of u1i to stdout. Empties u1o.	*/

void	u1pmp(char *pt) { int b;
	/* Puts the text of a message from pt to u1o interpreting
	   substitutions and special characters
	*/
	while ( (b=*pt++)!='\0' ) {
		if ( b >11 ) u1pc(b);   else
		if ( b==11 ) u1pc(7);   else
		if ( b==10 ) u1put(-1); else u1ps(b);
	}
}

#define u1pmi(i)        u1pmp(u1t+i);

void	u1pem(int m) { char *s; int i;
	/* Puts an error message in u1o */
	s=u1t; for ( i=0; i<m; i++ ) while ( *s++!='\0' ) ;
	u1pmp(s); u1put(m-1);
}


/* --- Get functions -------------------------------------------------- */
/* All u1g-functions return 0 upon failure, >0 upon success	*/

int	u1gl(FILE *f) {
	/* Fetches a line from file f into u1i. Returns ascii number of
	   first character or -1 when fetching failed.
	*/
	*u1i='\0';
	if ( fgets(u1i,u1imax,f)==NULL ) return -1;
/***** is this needed ??? ****/
	u1ip=u1i; while ( *u1ip!='\0' ) u1ip++;
	if ( u1ip-u1i>0 ) {
		u1ip--; if ( *u1ip=='\n' ) *u1ip='\0';
	}
	u1ir=u1ip=u1i;
	return (int)*u1ip;
}

char	u1get(void) { char *op; int c;
	/* Outputs content of u1o as a prompt (resetting it afterwards)
	   and fetches a line from user (either through u1fi when open,
	   or through stdinf otherwise) into u1i, setting u1ip to the
	   first character in u1i. Handles redirection requests and
	   writing to u1fo when appropriate. Also handles end-of-file
	   for u1fi and switching to stdin.
	   u1get fetches always a (possible empty) line. A '\0' is
	   appended to it.
	   Returns first character of line.
	*/
	op=u1op;
	do { 	u1op=op; u1put(0);
		c=-1;
		if ( u1fi!=NULL ) {
			c=u1gl(u1fi);
			if ( c<0 ) { fclose(u1fi); u1fi=NULL; }
		}
		if ( c<0 ) if ( u1gl(stdin)<0 ) u1stop();
		c=0; u1ip=u1i;
		if ( *u1ip=='<' ) {
			u1skp(1); c=( u1fi==NULL ? 1 : 0);
			if ( c ) if ( (u1fi=fopen(u1ip,"r"))==NULL ) c=-1;
		} else if ( *u1ip=='>') {
			u1skp(1); c=1;
			if ( u1fo!=NULL ) { fclose(u1fo); u1fo=NULL; }
			if ( *u1ip!='\0' )
				if ( (u1fo=fopen(u1ip,"w"))==NULL ) c=-1;
		}
		if ( c<0 ) putchar(7);
	} while ( c!=0 );
	if ( u1fi!=NULL ) u1pu();
	if ( u1fo!=NULL ) u1pl(u1fo,u1i,-1);
	u1ip=u1i;
	return *u1ip;
}

int	u1gb(int *v) { char b; int c;
	/* Fetches a boolean value from u1i into *b. No skip of leading
	   spaces. Returns 1 upon succes, 0 otherwise (with *b unchanged)
	*/
	c=1; b=u1upc(*u1ip);
	if ( b==u1cd[2] || b=='Y' ) *v=1;
	else if ( b==u1cd[3] || b=='N' ) *v=0; else c=0;
	if ( c>0 ) u1skp(1);
	return c;
}

int	u1gi(int *v) { char b; int c,s; long i;
	/* Fetches an integer from u1i into *v. No skip of leading
	   spaces. Returns 1 upon succes, 0 upon failure (with *v
	   unchanged).
	*/
	b=*u1ip;
	s=( b=='-' ? -1 : 1 );
	if ( b=='+' || b=='-' ) b=*(++u1ip);
	c=0; i=0;
	while ( u1type[b]==1 ) { i=10*i+(int)(b-'0'); c=1; b=*(++u1ip); }
	if ( c ) if ( i>32767L ) c=0;
	if ( c ) *v=s*(int)i;
	return c;
}

int	u1gd(double *d) { double d1,d2; char b; int c,i,k1,k2,s;
	/* Fetches a double from u1i into *v. No skip of leading
	   spaces. Returns 1 upon succes, 0 upon failure (with *v
	   unchanged).
	*/
	d1=0; k1=0;
	b=*u1ip;
	s=1; if ( b=='-' ) s=-1;
	if ( b=='+' || b=='-' ) b=*(++u1ip);
	while ( u1type[b]==1 ) { d1=10*d1+(b-'0'); k1++; b=*(++u1ip); }
	c=k1>0;
	if ( b=='.' ) {
		d2=1; k2=0; c=1; b=*(++u1ip);
		while ( u1type[b]==1 )
			{ d2*=0.1; d1+=(b-'0')*d2; k2++; b=*(++u1ip); }
		if ( k1+k2==0 ) return 0;
	}
	if ( u1upc(b)=='E' ) {
		if ( c==0 ) { c=1;  d1=1; }
		u1ip++; if ( u1gi(&i)==0 ) return 0;
		k1=k1+i;
		while ( i<0 ) { d1*=0.1; i++; }
		while ( i>0 ) { d1*= 10; i--; }
	}
	if ( c!=0 ) *d=s*d1;
	return c;
}

int	u1gt(char *t,int mt) { char b,q[5]; int i,lq;
	/* Fetches a text from u1i */
	q[1]='\0'; q[2]=' '; q[3]=','; q[4]=';'; lq=4;
	b=*u1ip;
	if ( b=='"' || b==(char)39 ) { q[2]=b; lq=2; b=*(++u1ip); }
	mt--;	/* must add '\0' always	*/
	while ( 1 ) {
		q[0]=b; i=lq; while ( q[i]!=b ) i--;
		if ( i>0 ) break;
		if ( mt>0 ) { *t++=b; mt--; }
		b=*(++u1ip);
	}
	*t='\0';
	if ( i==2 && q[2]!=' ' ) u1ip++;
	return 1;
}

int	u1gcmd1(void) { int l,t;
	/* Finds length of possible command at u1ip (u1ip unchanged).
	   Returns length of it.
	*/
	l=0;
	t=u1type[*u1ip];
	while ( t==2 ) {
		l++;
		t=u1type[*(u1ip+l)]; if ( t==1 && u1cd[1]!=' ' ) t=2;
	}
	return l;
}

int	u1gcmd(char *s,int ms) { int i,l;
	/* Fetches a possible command from u1i into s (max ms
	   characters). Returns length of s
	*/
	l=u1gcmd1(); if ( l>=ms ) l=ms-1;
	for ( i=0; i<l; i++ ) *s++=u1upc(*u1ip++);
	*s='\0';
	return l;
}


/* --- Substitute functions ------------------------------------------- */

void	u1sb(int s,int v) {
	/* Substitutes boolean v at place s */
	if ( s>0 && s<=10 ) { u1scl[s]=1; u1sbl[s]=v; }
}

void	u1si(int s,int v) {
	/* Substitutes integer v at place s */
	if ( s>0 && s<=10 ) { u1scl[s]=2; u1sil[s]=v; }
}

void	u1sd(int s,double v)  {
	/* Substitutes double precision v at place s */
	if ( s>0 && s<=10 ) { u1scl[s]=3; u1sdl[s]=v; }
}

void	u1st(int s,char *v) {
	/* Substitutes text v at place s	*/
	if ( s>0 && s<=10 ) {
		u1scl[s]=4; u1stl[s]=u1t;
		if ( u1tr<=u1tq ) {
			u1stl[s]=u1tr;
			while ( *v!='\0' && u1tr<u1tq ) *u1tr++=*v++;
			*u1tr++='\0';
		}
	}
}


/* --- Error functions ------------------------------------------------ */

void	u1e(int i,int obj) {
	/* Fatal error, no return */
	u1si(1,i); u1si(2,obj); u1pem(4); u1pem(5); u1stop();
}

void	u1ue(void) { char *p;
	/* Handles a non-fatal user error */
	u1pu();
	p=u1i;
	while ( p<u1ir ) *p++=' ';
	while ( p<u1ip ) *p++='^';
	if ( u1ip==u1ir ) *p++='^';
	*p='\0';
	u1pu();
	u1clr();
}


/* --- Miscellaneous functions ---------------------------------------- */

int	u1gp1(int c,int v) { int i,n,p,q;
	/* Returns the u1k-position of command v (c=0) or message v
	   (c=1). When position is not found, u1gp1 is set to 0.
	*/
	i=2*c+1; n=u1k[i+1]; p=u1k[i]-n; q=u1k[i+2];
	i=0;
	do { 	p+=n;
		if ( p>=q ) i=-1; else if ( u1k[p]==v ) i=1;
	} while ( i==0 );
	if ( i<0 ) p=0;
	return p;
}

int	u1gp(int c,int v) { int p;
	/* As u1gp1, but fatal error when position not found */
	if ( (p=u1gp1(c,v))==0 ) u1e(2*c+1,v);
	return p;
}

void	u1pm(int m) { u1pmi(u1k[u1gp(1,m)+2]); u1put(1); u1cs(); }
	/* Outputs a message */


/* --- Initialization functions --------------------------------------- */

#define u1ei(i)         { u1e(i+10,u1l); }
	/* Errors during u1ini */

void	u1gli(void) { u1l++; if ( u1gl(u1fc)<0 ) u1ei(1); }

void	u1free(void) {
	if ( u1k!=NULL ) free(u1k);
	if ( u1t!=NULL ) free(u1t);
	if ( u1h!=NULL ) free(u1h);
}

int	u1gui(void) { int i;
	/* Fetches an unsigned integer from field u1ip[0]..u1ip[3].
	   Returns unsigned value fetched or -1 when field empty.
	   Aborts when field invalid.
	*/
	u1skp(0); if ( u1ip-u1i>4 ) u1ip=u1i+4;
	if ( *u1ip=='\0' || u1ip-u1i==4 ) return -1;
	i=-2; u1gi(&i); if ( i<0 ) u1ei(4);
	u1skp(0); if ( u1ip-u1i>4 ) u1ip=u1i+4;
	return i;
}

void	u1addt(char b) {
	if ( u1tp>u1tq ) u1ei(3);
	*u1tp++=b;
}

int	u1addm(void) { char b;
	/* Converts message text in u1ip to an internal string in u1t,
	   converting escape sequences to special ascii values.
	   Returns 1 when text ends with ~ (tilde), 0 otherwise.
	*/
	do {	b=*u1ip++;
		if ( b=='~' ) {
			b=*u1ip++;
			if ( b=='\0' ) return 1;
			if ( u1type[b]==1 ) b-='0'; else
			if ( u1upc(b)=='N' ) b=(char)10; else
			if ( u1upc(b)=='B' ) b=(char)11;
		}
		u1addt(b);
	} while ( b!='\0' );
	return 0;
}

void	u1ini(char *fn) { char b; int a,i,k,kp,m,n,nc,nm,p,sp,sp1;
	/* Reads control file */

	/*=== Prolog ===					*/
	/* Establish u1type 					*/
	for ( i=0; i<128; i++ ) u1type[i]=3;
	for ( i=65; i<91; i++ ) u1type[i]=u1type[i+32]=2;
	for ( i=48; i<58; i++ ) u1type[i]=1;
	u1type[0]=-1; u1type[32]=0; u1type[95]=2;

	/* Establish user input and output buffers		*/
	u1oq=u1o+u1omax; u1clr();

	/* Create and initialize u1k and u1t			*/
	u1free();
	if ( (u1k=(int*)malloc(u1kmax*sizeof(int)))==NULL ) u1ei(1);
	if ( (u1t=(char*)malloc(u1tmax))==NULL ) u1ei(1);
	u1tp=u1t; u1tq=u1t+(u1tmax-1);

	/* Start with empty string in u1t (used	by		*/
	/* non-filled or overflowed text substitutes)		*/
	*u1tp++='\0';
	/* Copy error messages to u1t 				*/
	for ( i=1; i<5; i++ ) { u1em(i,u1i); u1ip=u1i; u1addm(); }

	/* Try to open control file 				*/
	if ( (u1fc=fopen(fn,"r"))==NULL ) u1ei(0);
	u1l=0;

	/*=== Header section ===				*/
	/* First line 						*/
	u1gli();
	for ( i=0; i<4; i++ ) if ( (b=*u1ip)!='\0' ) {
		if ( b!=' ' && b!='0' ) u1cd[i]=u1upc(b);
		u1ip++;
	}
	/* Second line. Copy as error message 5 to u1t		*/
	u1gli(); u1addm();

	/*=== Command section ===				*/
	kp=6; nc=0;
	u1k[1]=kp; u1k[2]=6; sp=1; u1s[1]=0;
	while ( 1 ) {
		/* Fetch next line				*/
		u1gli(); k=u1gui(); if ( k<0 ) break;

		/* Next command */
		if ( kp+u1k[2]>u1kmax ) u1ei(2);
		u1k[kp]=k; u1k[kp+1]=-1; u1k[kp+3]=u1k[kp+4]=0;

		b=*u1ip; a=0;
		if ( b=='-' ) a=1; else
		if ( b=='!' ) a=2; else
		if ( b=='/' ) a=3;

		/* Detect level sp1				*/
		u1skp(1); sp1=(int)(u1ip-u1i)-5;
		if ( sp1>sp+1 || sp1>u1smax ) u1ei(6);

		n=u1gcmd1(); if ( n==0 || n>31 ) u1ei(7);
		u1k[kp+2]=(int)(u1tp-u1t);
		u1k[kp+5]=(a<<6)+n;
		while ( --n>=0 ) u1addt(*u1ip++);
		b=*u1ip++;
		if ( b=='+' ) {
			n=u1gcmd1();
			while ( --n>=0 ) u1addt(*u1ip++);
		} else if ( b=='-' ) {
			u1k[kp+5]+=32;
			while ( *u1ip!='\0' ) u1addt(*u1ip++);
		} else u1ip--;
		u1addt('\0');
		if ( *u1ip!='\0' ) u1ei(7);

		/* Update structure information			*/
		if ( sp1<sp ) sp=sp1;
		p=u1s[sp]; if ( p>0 ) u1k[p+(sp1-sp+3)]=kp;
		sp=sp1; u1s[sp]=kp;
		kp+=u1k[2]; nc++;
	}

	/*=== Message section ===				*/
	nm=0;
	u1k[3]=kp; u1k[4]=3;
	n=0;
	while ( 1 ) {
		/* Fetch next line				*/
		u1gli();
		if ( n==0 ) {
			/* Not a continuation line.		*/
			m=u1gui();
			if ( m<0 ) break;
			if ( kp+u1k[4]>u1kmax ) u1ei(2);
			u1k[kp]=m; u1k[kp+1]=-1; u1k[kp+2]=(int)(u1tp-u1t);
			kp+=u1k[4]; nm++;
		} else u1skp(0);
		n=u1addm();
	}
	if ( n>0 ) u1addt('\0');

	/*=== Epilog ===					*/
	u1k[0]=u1k[5]=kp;

	/* Create help position area u1h			*/
	u1h=(long*)malloc((unsigned)((nc+nm+1)*sizeof(long)));
	if ( u1h==NULL ) u1ei(1);
	u1hp=0;

	/* No input or output redirection 			*/
	u1fi=u1fo=NULL;

	u1sp=0; u1clr();
}

void	u1fini(void) {
	u1free();
	if ( u1fi!=NULL ) fclose(u1fi);
	if ( u1fo!=NULL ) fclose(u1fo);
	if ( u1fc!=NULL ) fclose(u1fc);
	u1fc=u1fi=u1fo=NULL;
}


/* --- Help functions ------------------------------------------------- */

void	u1hlp0(void) { int i,n,p,q;
	/* Sets all unfilled help links (u1k[p+1]<0) to not available
	   (u1k[p+1]=0).
	*/
	i=1;
	while ( i<=3 ) {
		p=u1k[i++]; n=u1k[i++]; q=u1k[i];
		p++;
		while ( p<q ) {
			if ( u1k[p]<0 ) u1k[p]=0;
			p+=n;
		}
	}
}

void	u1hlp1(void) { char b; int c,i,n,p,q; long pos;
	/* Current help-file line starts with <.
	   Tries to establish a pointer to help information
	*/
	b=u1upc(u1skp(1));
	c=-1; if ( b=='C' ) c=0; else if ( b=='M' ) c=1;
	if ( c>=0 ) {
		/* We found C (c=0) or M (c=1).			*/
		/* Try to fetch a number			*/
		u1skp(1);
		i=-1; u1gi(&i); if ( i<0 ) return;
		c=2*c+1; p=u1k[c]; n=u1k[c+1]; q=u1k[c+2];
		pos=ftell(u1fc);
		for ( ; p<q; p+=n )
			if ( u1k[p]==i ) if ( u1k[p+1]<0 )
				{ u1hp++; u1k[p+1]=u1hp; u1h[u1hp]=pos; }
	}
}

void	u1hlp(int p) { char *ip,*ir,*jp; int b,c;
	/* Tries to find help and display information. Action depends
	   on value of help-text pointer u1k[p+1]
	*/

	/* Save u1i in u1j			*/
	ip=u1i ; jp=u1j ; while ( (*jp++=*ip++)!=0 ) ;
	ip=u1ip; ir=u1ir;

	while ( u1k[p+1]<0 ) {
		b=u1gl(u1fc);
		if ( b<0 ) u1hlp0(); else
		if ( (char)b=='<' ) u1hlp1();
	}
	if ( u1k[p+1]>0 ) {
		c=( fseek(u1fc,u1h[u1k[p+1]],SEEK_SET) ? 0 : 1 );
		while ( c ) {
			b=u1gl(u1fc); if ( b<0 ) break;
			if ( (char)b=='>' ) {
				c=u1skp(1); u1gb(&c); u1sb(1,c);
				u1pem(1); u1get(); u1gb(&c); u1clr();
			} else if ( (char)b=='<' ) {
				u1hlp1(); c=0;
			} else u1pu();
		}
	} else u1pem(2);

	/* Restore u1i from u1j			*/
	u1ip=ip; u1ir=ir;
	ip=u1i ; jp=u1j ; while ( (*ip++=*jp++)!=0 ) ;
}

void	u1hlpc(int k) { u1hlp(u1gp(0,k)); }
	/* Give help for command k */

void	u1hlpm(int m) { u1hlp(u1gp(1,m)); }
	/* Gives help for message m */


/* --- Command functions ---------------------------------------------- */

void	u1sc1(int pc) { int i,j,n,n1,p,pt;
	/* Show Commands routine controlled by pointer-to-command pc */

	if ( (p=u1k[pc+4])==0 ) u1e(2,u1k[p]);
	/* Establish n to be longest abbreviation length */
	n=0;
	while ( p>0 ) {
		if ( (n1=u1k[p+5]&31)>n ) n=n1;
		p=u1k[p+3];
	}

	u1pc(u1cd[0]); u1put(-1);
	p=u1k[pc+4];
	while ( p>0 ) {
		pt=u1k[p+2]; j=u1k[p+5]&63; n1=j&31; i=0;
		while ( i<n1 ) u1pc(u1t[pt+(i++)]);
		while ( i++<=n  ) u1pc(' ');
		u1pc('-'); u1pc(' ');
		if ( j>32 ) pt=pt+n1;
		u1pt(u1t+pt); u1put(-1);
		p=u1k[p+3];
	}
}

int	u1cmp(char *a,int la,char *b) {
	/* Compares (non-empty) strings a and b.
	   Returns 0 upon failure, 1 upon succes.
	*/
	while ( 1 ) {
		if ( --la<0 ) return 1;
		if ( *b=='\0' ) return 0;
		if ( u1upc(*a++)!=u1upc(*b++) ) return 0;
	}
}

int	u1fndc(int p) { int d,l,q;
	/* Searches subcommands of command pointer p for match with u1ip.
	   Returns command pointer of match when found, or 0 when no
	   match found.
	*/
	l=u1gcmd1(); if ( l==0 ) return 0;
	q=u1k[p+4];
	while ( 1 ) {
		if ( q==0 ) break;
		d=u1k[q+5]&63;
		if ( d>32 ? l==d-32 : l>=d )
			if ( u1cmp(u1ip,l,u1t+u1k[q+2])>0 ) break;
		q=u1k[q+3];
	}
	u1ip+=l;
	return q;
}

void	u1pget(int p) { char *t; int i;
	/* Prompt with command name  and fetch new user input	*/
	i=(u1k[p+5]&63)-32; t=u1t+u1k[p+2];
	if ( i<0 ) u1pt(t);
	while ( i>0 ) { u1pc(*t++); i--; }
	u1pc('>');
	u1get();
}

int	u1key1(int mk,int onec,int rpte) { char b; int c,d,h,p,q,s,sh,sp;
	/* Command scanning routine */

	/* Skip possible semicolon 				*/
	if ( *u1ip==';' ) u1skp(1);

	/* Pick-up valid stack environment 			*/
	if ( u1sp>0 ) if ( u1k[u1s[1]]!=mk ) u1sp=0;
	if ( u1sp==0 ) { u1sp=1; u1s[1]=u1gp(0,mk); }
	sp=u1sp; p=u1s[sp];

	if ( u1k[p+4]==0 ) u1e(2,u1k[p]);

	/* h=help, s=show menu, c=control (aimed to become!=0)	*/
	/* sh=stack level when asking help			*/
	h=s=c=sh=0;
	while ( c==0 ) {
		while ( (b=u1skp(0))=='\0' ) {
			/* We need a new input line 		*/
			/* Help needed ? 			*/
			if ( h>0 && u1fi==NULL )
				{ u1hlp(p); p=u1s[sp=sh]; s=0; }
			/* Menu needed ? 			*/
			if ( s>0 && u1fi==NULL ) u1sc1(p);
			h=0; s=1;
			u1pget(p);
		}

		q=-1; s=1;
		if ( b==u1cd[0] ) { sh=sp; h=1; } else
		if ( b=='/' ) { u1sp=sp=1; p=u1s[1]; } else
		if ( b=='!' ) {
			if ( sp>1 ) sp--;
			if ( u1sp>sp ) u1sp=sp;
			p=u1s[sp];
		} else {
			/* Find subcommand of p			*/
			q=u1fndc(p);
		}

		if ( q<0 ) {
			/* Just handled special character	*/
			u1skp(1); s=0;
		} else if ( q>0 ) {
			/* Match found */
			p=q; c=1; d=u1k[p+5]>>6;
			if ( u1k[p+4]>0 ) {
				/* There are subcommands	*/
				sp++; u1s[sp]=p;
				if ( d==1 ) u1sp=sp;
				/* continue when onec==0 	*/
				if ( onec==0 ) c=0;
			} else {
				if ( d==2 ) u1sp--; else
				if ( d==3 ) u1sp=1;
				if ( d>=2 ) { p=u1s[sp=u1sp]; c=s=0; }
			}
		} else {
			/* q==0 : match failed			*/
			/* Exit with -1 when rpte<>0, issue	*/
			/* soft error and stay when rpte=0	*/
			if ( h==0 && rpte ) c=-1; else { u1ue(); h=0; }
		}
		/* Stay when help requested (h>0) */
		if ( h>0 ) c=0;
	}
	/* Set key-value */
	return ( c>0 ? u1k[p] : -1 );
}

int	u1key(int mk)       { return u1key1(mk,0,0); }


/* --- Ask functions -------------------------------------------------- */

int	u1aq(int m) { char b; int c,p;
	/* Ask prolog for all ask routines. Outputs prompt and gives
	   help when necessary; sets return code c:
	      c =  1 : value available to be fetched by ask routine
		   0 : default value should be used
		  -1 : something happened which requires repeating the
		       ask sequence (either help given or no value
		       available where no default is accepted due to m<0)
	*/
	p=u1gp(1,abs(m));
	if ( (b=u1skp(0))==0 ) {
		u1pmi(u1k[p+2]); u1pc(' ');
		if ( m>0 ) { u1pc('('); u1ps(10); u1pc(')'); }
		u1pc(':'); u1pc(' '); u1get(); u1cs(); b=u1skp(0);
	}
	c=0;
	if ( b==u1cd[0] ) { c=-1; if ( u1fi==NULL ) u1hlp(p); }
	else if ( b!='\0' && b!=',' && b!=';' ) c=1;
	else if ( m<0 ) { c=-1; if ( b!='\0' ) u1ue(); }
	if ( c==0 ) if ( b==',' ) u1skp(1);
	return c;
}

#define	ask(subst,get) { int c;					\
	/* Asks a value */					\
	do {	subst(10,v);					\
		c=u1aq(m);					\
		if ( c>0 ) { c=get(&v)-1; if ( c<0 ) u1ue(); }	\
	} while ( c<0 );					\
}

int	u1ab(int m,int v) { ask(u1sb,u1gb); return v; }
	/* Asks a boolean value */

int	u1ai(int m,int v) { ask(u1si,u1gi); return v; }
	/* Asks an integer value */

double	u1ad(int m,double v) { ask(u1sd,u1gd); return v; }
	/* Asks a double precision value */

void	u1at(int m,char *v,int mv) { int c;
	/* Asks a text value */
	do { 	u1st(10,v);
		c=u1aq(m); if ( c>0 ) u1gt(v,mv);
	} while ( c<0 );
}

#define	askb(subst,ask) { 					\
	if ( s==0 && v1>v2 ) {					\
		subst(1,v1); subst(2,v2); u1pem(3); u1e(4,m);	\
	}							\
	r=ask(m,v);						\
	if ( s>=0 )						\
	while ( (r<v1 && s<2) || (r>v2 && s%2==0) ) {		\
		u1ue();						\
		if ( s<2 ) subst(1,v1);				\
		if ( s%2==0 ) subst(2,v2);			\
		u1pem(3);					\
		r=ask(m,v);					\
	}							\
}

int	u1aib(int m,int v,int v1,int v2,int s) { int r;
	/* Asks a bounded integer value */
	askb(u1si,u1ai); return r;
}

double	u1adb(int m,double v,double v1,double v2,int s) { double r;
	/* Asks a bounded double precision value */
	askb(u1sd,u1ad); return r;
}


/* --- Miscellaneous routines for external usage ---------------------- */

int	u1more(void) { return ( *u1ip=='\0' ? 0 : -1 ); }
	/* Type-ahead data available? */

void	u1sc(int k) { u1sc1(u1gp(0,k)); }
	/* Show subcommands of command k */

void	u1memc(int *nk,int *nt) { *nk=u1k[0]; *nt=(int)(u1tp-u1t); }
	/* MEMory, Current values */

void	u1memm(int *nk,int *nt) { *nk=u1kmax; *nt=u1tmax; }
	/* MEMory, Maximum values */

void	u1mems(int nk,int nt) {
	/* MEMory, Set maximum values */
	if ( u1t==NULL ) { u1kmax=nk; u1tmax=nt; }
}

/*	end of u1.c	*/
