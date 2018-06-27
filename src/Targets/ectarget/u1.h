/*	u1.h
	====

*/

#ifndef	u1_h
#define	u1_h

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize routines */
void	u1ini(char *fn);
void	u1fini(void);
/* command routines */
int	u1key1(int mk,int onec,int rpte);
int	u1key(int mk);
/* Ask routines */
int	u1ab(int m,int v);
int	u1ai(int m,int v);
double	u1ad(int m,double v);
void	u1at(int m,char *v,int mv);
int	u1aib(int m,int v,int v1,int v2,int s);
double	u1adb(int m,double v,double v1,double v2,int s);
/* Put routines */
void	u1put(int eol);
void	u1pb(int b);
void	u1pi(int i);
void	u1pd(double d);
void	u1pt(char *t);
void	u1pm(int m);
/* Get routines */
char	u1get(void);
int	u1gb(int *b);
int	u1gi(int *i);
int	u1gd(double *d);
int	u1gt(char *t,int mt);
int	u1gcmd(char *s,int ms);
/* Substitute routines */
void	u1sb(int s,int v);
void	u1si(int s,int v);
void	u1sd(int s,double v);
void	u1st(int s,char *v);
/* Menu and help routines */
void	u1sc(int k);
void	u1hlpc(int k);
void	u1hlpm(int m);
/* Misc routines */
void	u1clr(void);
int	u1more(void);
char	u1skp(int n);
void	u1back(void);
void	u1memc(int *nk,int *nt);
void	u1memm(int *nk,int *nt);
void	u1mems(int nk,int nt);

#ifdef __cplusplus
}
#endif

#endif

/*	end of u1.h	*/
