
typedef int (*PTIMERCALLBACK) (int,int);

#ifdef __cplusplus
extern "C" {
#endif
double rtc_get_time(void);
double rtc_get_nominal_time(void);
int rtc_timer(double, PTIMERCALLBACK,int);
int rtc_usleep(int);
int rtc_sleep(int);
#ifdef __cplusplus
}
#endif

typedef struct tag_timer_stat
{
    int icnt;
    int imissed;
    double avg_cpu_time;
    double max_cpu_time;
    double max_err;
    double dt;
} timer_stat, *ptimer_stat;

#ifdef __cplusplus
extern "C" {
#endif
int rtc_get_timer_stat(ptimer_stat);
#ifdef __cplusplus
}
#endif
