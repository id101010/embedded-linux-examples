#include<time.h>
#include<sys/time.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>

time_t timerid_1, timerid_2, timerid_3, timerid_4;
struct sigevent se_timer1, se_timer2, se_timer3, se_timer4;
struct itimerspec ts_1, ts_2, ts_3, ts_4;

volatile static int32_t counter1, counter2, counter3, counter4 = 0;

/* Callback functions -----------------------------------------*/

/* Timer1 callback */
void callback_1(union sigval arg)
{
    printf("Counter_1 value: %d\n", counter1++);
}

/* Timer2 callback */
void callback_2(union sigval arg)
{
    printf("Counter_2 value: %d\n", counter2++);
}

/* Timer1 callback */
void callback_3(union sigval arg)
{
    printf("Counter_3 value: %d\n", counter3++);
}

/* Timer2 callback */
void callback_4(union sigval arg)
{
    printf("Counter_4 value: %d\n", counter4++);
}


/* Internal functions -----------------------------------------*/

/* Timer initialization */
void init_timer(void (*callback)(union sigval arg), 
                struct sigevent *se, 
                struct itimerspec *ts,
                time_t *timerid, 
                int nanoseconds,
                int seconds)
{

    /* Setup signal handling and callback for timer 1 */
    se->sigev_notify             = SIGEV_THREAD;
    se->sigev_value.sival_ptr    = timerid;
    se->sigev_notify_function    = callback;
    se->sigev_notify_attributes  = NULL;
    
    /* Create the timer and check for any errosr */
    if (timer_create(CLOCK_REALTIME, se, timerid) == -1) {
        perror("timer_create:");
        return(1);
    }

    /* Init timer values */
    ts->it_value.tv_sec      = 1;
    ts->it_value.tv_nsec     = 0;
    ts->it_interval.tv_sec   = seconds;
    ts->it_interval.tv_nsec  = nanoseconds;
}

/* Start timer */
void start_timer(struct itimerspec *ts, time_t *timerid)
{
    /* Set the timer and check for any errors */
    if (timer_settime(*(timerid), 0, ts, NULL) == -1) {
        perror("timer_settime:");
    }
}

/* ------------------------------------------------------------*/

/* Main function */
int main (int argc, char **argv)
{
    /* Init timers */
    init_timer(callback_1, &se_timer1, &ts_1, &timerid_1, 0, 1);
    init_timer(callback_2, &se_timer2, &ts_2, &timerid_2, 500000000, 0);
    init_timer(callback_3, &se_timer3, &ts_3, &timerid_3, 250000000, 0);
    init_timer(callback_4, &se_timer4, &ts_4, &timerid_4, 125000000, 0);
    
    /* Start Timers */
    start_timer(&ts_1, &timerid_1);
    start_timer(&ts_2, &timerid_2);
    start_timer(&ts_3, &timerid_3);
    start_timer(&ts_4, &timerid_4);

    /* Endless loop */
    while (1) { 
        ;
    }

    return EXIT_SUCCESS;
}
