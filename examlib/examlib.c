/*
 ***************************************************************************
 * \brief   Embedded Linux examlib
 *
 *          Potentiometer:
 *          --------------
 *          Basic analog to digital converter (ADC) application.
 *          Convert Analog voltage on AIN4 (Potentiometer) to Digital value.
 *
 *          D = Vin * (2^N - 1) / Vref
 *
 *          Where:
 *          D = Digital value
 *          Vin = Input voltage
 *          N = No of bits (12-Bits)
 *          Vref = reference voltage (1.8V)
 *
 *          LEDs:
 *          -----
 *
 *          Buttons:
 *          --------
 *
 *          Timers:
 *          -------
 *
 * \file    examlib.c
 * \version 1.0
 * \date    17.01.2016
 * \author  Schmocker Aaron
 *
 * \remark  Last Modifications:
 * \remark  V1.0, SCHMA5, 17.01.2016   Initial release
 ***************************************************************************
 *
 * Copyright (C) 2016 Aaron Schmocker, Bern University of Applied Scinces
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<signal.h>
#include<math.h>
#include<time.h>
#include<signal.h>
#include<fcntl.h>
#include<string.h>
#include<stdbool.h>
#include<errno.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/time.h>

/* String to access the ADC4 via sysfs */
#define AIN4_DEV	    "/sys/bus/iio/devices/iio:device0/in_voltage4_raw"

/* Delay value for one second in us */
#define ONE_SECOND		1000000

/* Define some useful constants */
#define BUFFER_SIZE		16
#define ADC_BIT_RES		12
#define V_REF			1.8
#define N				pow(2,12)
#undef DEBUG

/* Delay value in micro seconds */
#define ONE_MILLISECOND	1000
#define ONE_SECOND		1000000

/* Define some useful constants */
#define SYSFS_PATH		"/sys/class/gpio/"
#define MAX_STR_BUF		512
#define MAX_PATH_STR	512
#define LOW		    	0
#define HIGH			1
#define OUTPUT			0
#define INPUT			1
#define PRESSED			'0'
#define LED_ON          0
#define LED_OFF         1

/* Define the GPIO numbers of the BBB-BFH-Cape LEDs */
#define LED_1			61
#define LED_2			44
#define LED_3			68
#define LED_4			67

/* Define the assignment button to pin number of the BBB-BFH-Cape */
#define BUTN_1			49
#define BUTN_2			112
#define BUTN_3			51
#define BUTN_4			7
#define MAX_GPIO		(4)

/* Define index of LEDs and Buttons */
#define L1	    0
#define L2	    1
#define L3		2
#define L4		3
#define T1		0
#define T2		1
#define T3		2
#define T4		3

/* Define some useful sysfs constants */
enum {
    GPIO_SYSFS_EXPORT = 0,
    GPIO_SYSFS_UNEXPORT,
    GPIO_SYSFS_GET_DIRECTION,
    GPIO_SYSFS_SET_DIRECTION,
    GPIO_SYSFS_GET_VALUE,
    GPIO_SYSFS_SET_VALUE,
} GPIO_ACTIONS;

/* Static variables */
static int32_t gpio_led[MAX_GPIO] = {LED_1, LED_2, LED_3, LED_4};
static int32_t gpio_btn[MAX_GPIO] = {BUTN_1, BUTN_2, BUTN_3, BUTN_4};
static char ON[]  = "0";
static char OFF[] = "1";
static char OUT[] = "out";
static char IN[]  = "in";

/* Other variables */
char  	adcBuffer[BUFFER_SIZE];
char   	button[3];
char   	led[3];
time_t 	timerid_1, timerid_2, timerid_3, timerid_4, timerid_adc, timerid_btn;
struct 	sigevent se_timer1, se_timer2, se_timer3, se_timer4, se_timer_adc, se_timer_btn;
struct 	itimerspec ts_1, ts_2, ts_3, ts_4, ts_adc, ts_btn;
volatile int32_t counter1, counter2, counter3, counter4 = 0;

/* Prototypes */
void callback_1(union sigval arg);
void callback_2(union sigval arg);
void callback_3(union sigval arg);
void callback_4(union sigval arg);
void callback_adc(union sigval arg);
void callback_btn(union sigval arg);
void init_timer(void (*callback)(union sigval arg), struct sigevent *se, struct itimerspec *ts, time_t *timerid, int nanoseconds, int seconds);
void start_timer(struct itimerspec *ts, time_t *timerid);
void signal_ctrlc_handler(int signum);
void signal_terminate_handler(int signum);
int sysfs_gpio_handler(uint8_t function, uint32_t gpio, char *val);
float read_adc_value(void); 

/* Timer1 callback */
void callback_1(union sigval arg)
{
    sysfs_gpio_handler(GPIO_SYSFS_GET_VALUE, gpio_led[L1], &led[L1]);   // Read value

    //printf("L1: %d \n", led[L1]);

    if(led[L1] == '0'){                                                 // Toggle
        sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[L1], OFF);
    }
    if(led[L1] == '1'){
		sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[L1], ON);
    }
}

/* Timer2 callback */
void callback_2(union sigval arg)
{
    sysfs_gpio_handler(GPIO_SYSFS_GET_VALUE, gpio_led[L2], &led[L2]);   // Read value

    //printf("L1: %d \n", led[L1]);

    if(led[L2] == '0'){                                                 // Toggle
        sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[L2], OFF);
    }
    if(led[L2] == '1'){
		sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[L2], ON);
    }
}

/* Timer1 callback */
void callback_3(union sigval arg)
{
    sysfs_gpio_handler(GPIO_SYSFS_GET_VALUE, gpio_led[L3], &led[L3]);   // Read value

    //printf("L1: %d \n", led[L1]);

    if(led[L3] == '0'){                                                 // Toggle
        sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[L3], OFF);
    }
    if(led[L3] == '1'){
		sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[L3], ON);
    }
}

/* Timer2 callback */
void callback_4(union sigval arg)
{
    sysfs_gpio_handler(GPIO_SYSFS_GET_VALUE, gpio_led[L4], &led[L4]);   // Read value

    //printf("L1: %d \n", led[L1]);

    if(led[L4] == '0'){                                                 // Toggle
        sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[L4], OFF);
    }
    if(led[L4] == '1'){
		sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[L4], ON);
    }
}

/* ADC_Timer callback */
void callback_adc(union sigval arg)
{
	float aValue = read_adc_value();
	sprintf(adcBuffer, "%f", aValue);
	printf("AIN4: %sV\n", adcBuffer);	// Write output to console
}

/* Button polling timer callback */
void callback_btn(union sigval arg)
{
    int i; 

	// Read button values
	for(i = 0; i < 4; i++){
		sysfs_gpio_handler(GPIO_SYSFS_GET_VALUE, gpio_btn[i], &button[i]);
	}

	// Set led values
	for(i = 0; i < 4; i++){
		if(button[i] == PRESSED){
			sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[i], ON);
		} else {
			sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[i], OFF);
		}
	} 
}

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

/* sysfs_gpio_handler - handle GPIO operations */
int sysfs_gpio_handler(uint8_t function, uint32_t gpio, char *val)
{
    char     path_str[MAX_PATH_STR];
    char     strBuf[MAX_STR_BUF];
    uint8_t  oflags = 0;
    int32_t  fd;
    uint32_t len;
    uint8_t  inval;

    /* Determine open flags based on function */
    switch (function) {
    case GPIO_SYSFS_EXPORT:
        snprintf(path_str, sizeof(path_str), SYSFS_PATH"export");
        oflags=O_WRONLY;
        break;

    case GPIO_SYSFS_UNEXPORT:
        snprintf(path_str, sizeof(path_str), SYSFS_PATH"unexport");
        oflags=O_WRONLY;
        break;

    case GPIO_SYSFS_SET_DIRECTION:
        snprintf(path_str, sizeof(path_str), SYSFS_PATH"gpio%d/direction", gpio);
        oflags=O_WRONLY;
        break;

    case GPIO_SYSFS_SET_VALUE:
        snprintf(path_str, sizeof(path_str), SYSFS_PATH"gpio%d/value", gpio);
        oflags=O_WRONLY;
        break;

    case GPIO_SYSFS_GET_DIRECTION:
    case GPIO_SYSFS_GET_VALUE:
        snprintf(path_str, sizeof(path_str), SYSFS_PATH"gpio%d/value", gpio);
        oflags=O_RDONLY;
        break;

    default:
        printf("File operation flag not defined\n");
    }

    /* Open the pseudo file given its path and open flags	*/
#ifdef DEBUG
    printf("open:%s\n", path_str);
#endif

    fd = open(path_str, oflags);
    if (fd  < 0) {
        perror(path_str);
        return fd;
    }

    /* File operations r/w on the opened file */
    switch (function) {
    case GPIO_SYSFS_EXPORT:
    case GPIO_SYSFS_UNEXPORT:
        len = snprintf(strBuf, sizeof(strBuf), "%d", gpio);
#ifdef DEBUG
        printf("exp/unexp:%s\n", strBuf);
#endif
        write(fd, strBuf, len);
        break;

    case GPIO_SYSFS_SET_DIRECTION:
#ifdef DEBUG
        printf("write dir:%s\n", val);
#endif
        write(fd, val, strlen(val)+1);
        break;

    case GPIO_SYSFS_SET_VALUE:
#ifdef DEBUG
        printf("write val:%s\n", val);
#endif
        write(fd, val, strlen(val)+1);
        break;

    case GPIO_SYSFS_GET_DIRECTION:
        break;

    case GPIO_SYSFS_GET_VALUE:
        read(fd, &inval, 1);
#ifdef DEBUG
        printf("read val:%c\n", inval);
#endif
        *val = inval;
        break;

    default:
        printf("function not defined\n");
    }
    close(fd);
    return 0;
}

/* init gpio ports */
void init_gpio(void)
{
    int i; 

    /* Setup gpio sysfs for the LEDs [L1..L4] and Buttons [T1..T4] */
    for (i=0; i<MAX_GPIO; i++) {
        sysfs_gpio_handler(GPIO_SYSFS_EXPORT, gpio_led[i], NULL);
        sysfs_gpio_handler(GPIO_SYSFS_SET_DIRECTION, gpio_led[i], OUT);
        sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[i], OFF);
        sysfs_gpio_handler(GPIO_SYSFS_EXPORT, gpio_btn[i], NULL);
        sysfs_gpio_handler(GPIO_SYSFS_SET_DIRECTION, gpio_btn[i], IN);
    }
}

float read_adc_value(void) 
{
	int charRead;
    int32_t adc_fd = open(AIN4_DEV, O_RDONLY);
    char adc_buffer[BUFFER_SIZE];

    if(adc_fd  < 0){
      perror("Dev not readable");
      return adc_fd;
    }

    charRead = read(adc_fd, adc_buffer, sizeof(adc_buffer));
    close(adc_fd);
    
	if (charRead != -1){
      adc_buffer[charRead] = '\0';
      float aValue = (float)atoi(adc_buffer);
      aValue = (V_REF * aValue) / ((1<<12)-1);

      return aValue;
    }

    return -1;
}

/* Handle sigint via ctrl c*/
void signal_ctrlc_handler(int signum)
{
	int i;
    printf("\nExit via Ctrl-C\n\n"); 	// Inform user

    /* Unexport all selected gpios */
    for (i=0; i<MAX_GPIO; i++) {
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_led[i], NULL);
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_btn[i], NULL);
    }

    exit(signum);						// Terminate
}

/* Handle sigint via term*/
void signal_terminate_handler(int signum)
{
	int i;
    printf("\nExit via Ctrl-C\n\n"); 	// Inform user

    /* Unexport all selected gpios */
    for (i=0; i<MAX_GPIO; i++) {
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_led[i], NULL);
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_btn[i], NULL);
    }

    exit(signum);						// Terminate
}


int main(int argc, char *argv[])
{    
	sigset_t 	set;

	/* SIGTERM handling */
    signal(SIGINT, signal_ctrlc_handler); 				// register signal handler
    signal(SIGTERM, signal_terminate_handler);			// Register SIGTERM signal handler
    sigemptyset(&set);									// Initializes the signalmask to empty
    sigaddset(&set, SIGALRM );							// Set the signal mask for the signal handler

	/* Init timers */
    init_timer(callback_adc, &se_timer_adc, &ts_adc, &timerid_adc, 0, 1);			// adc polling timer
    init_timer(callback_btn, &se_timer_btn, &ts_btn, &timerid_btn, 500000000, 0);	// button polling timer
    init_timer(callback_1, &se_timer1, &ts_1, &timerid_1, 0, 1);		
    init_timer(callback_2, &se_timer2, &ts_2, &timerid_2, 500000000, 0);
    init_timer(callback_3, &se_timer3, &ts_3, &timerid_3, 250000000, 0);
    init_timer(callback_4, &se_timer4, &ts_4, &timerid_4, 125000000, 0);
    
    /* Start Timers */
    start_timer(&ts_adc, &timerid_adc);
    //start_timer(&ts_btn, &timerid_btn);
    start_timer(&ts_1, &timerid_1);
    start_timer(&ts_2, &timerid_2);
    start_timer(&ts_3, &timerid_3);
    start_timer(&ts_4, &timerid_4);

	init_gpio(); // init buttons and leds


    while (1) {
		;
    }

    return 0;
}


