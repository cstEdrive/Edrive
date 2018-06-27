/*	tu1a.c
	======

	Tests basic functionality of C implementation of coin in u1.c
*/

#include <stdio.h>
#include <string.h>
#include "u1.h"

extern	FILE	*u1fdi,*u1fdo;
extern	char	*u1ip;
int	u1gl(FILE*);
void	u1pl(FILE*,char*,int);
void	u1em(int,char*);
void	u1stop(void);


char	fn[127],line[127];


void	wait(void) {
	printf("----> Press ENTER"); getchar();
}

void	out(char *line,int c,int where) {
	u1pl(stdout,line,c);
	if ( where ) u1pl(u1fdo,line,c);
}

void	doall(void) {
	int  i,j;

	strcpy(fn,"TU1A.TST");

	/* Try to open logging file TU1A.TST */
	if ( (u1fdo=fopen(fn,"w"))==NULL ) {
		puts("**** File TU1A.TST not opened for writing");
		return;
	}

	/* Prompt for and fetch user-line */
	printf("Type a line: ");
	u1gl(stdin); strcpy(line,u1ip);

	printf("\n");
	printf("----> You should get at consequitive lines:\n");
	printf("---->     0: - the line you just typed\n");
	printf("---->     1: - again the line you just typed\n");
	printf("---->     2: - your line, space, you line\n");
	printf("---->     3: - an empty line\n");
	printf("----> 4.. 6: - Readable ascii sequence, 32 per line\n");
	printf("----> 7..10: - Build-in messages\n");
	printf("----> When something seems wrong, please refer to the "
	       " TU1A.C source code\n");

	/* --- Registrate all lines in log-file --- */
	/* Output line 0 */
	out(line,1,1);
	/* Filter user-line through u1ord and u1chr */
	/* Output line 1 */
	out(line,1,1);

	/* Output line 2 */   /* Do not use eol=0 for f=2 */
	out(line,0,1); out(" ",0,1); out(line,1,1);
	/* Output line 3 */
	line[0]='\0'; out(line,1,1);

	/* Output lines 4..6 */
	for ( j=1; j<4; j++ ) {
		for ( i=0; i<32; i++ ) line[i]=(char)(32*j+i);
		if ( j==3 ) line[31]=' ';
		line[32]='\0';
		out(line,1,1);
	}

	/* Output lines 7..10 */
	for ( j=1; j<5; j++ ) { u1em(j,line); out(line,1,1); }
	fclose(u1fdo);


	wait();
	printf("----> All 11 lines again using command-file input\n");
	if ( (u1fdi=fopen(fn,"r"))==NULL ) {
		puts("**** File TU1A.TST not opened for reading");
		return;
	}
	while ( u1gl(u1fdi)>=0 ) out(u1ip,1,0);
	fclose(u1fdi);

	printf("----> I made file TU1A.TST which may be discarded\n");
}

int main(void) {
	doall();
	u1stop();
	puts("**** u1stop failed\n");
	return 0;
}

/*	end of tu1a.c	*/
