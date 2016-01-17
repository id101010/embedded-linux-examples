/*
 ***************************************************************************
 * \brief   Embedded-Linux (BTE5446)
 *	        Linux Sysfs GPIO Exercise 1.0
 *    
 * \file    appSysfsTemplate.c
 * \version 1.0
 * \date    17.01.2016
 * \author  Aaron Schmocker
 *
 * \remark  Last Modifications:
 * \remark  V1.0, AOM1, 25.10.2013      Initial release
 * \remark  V1.1, SCHMA5, 20.11.2015    Added POSIX Timer Handling
 ***************************************************************************
 *
 * Copyright (C) 2013 Martin Aebersold, Bern University of Applied Scinces
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

/* Declare the function prototypes headers */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#undef DEBUG

/* Delay value in micro seconds */
#define ONE_MILLISECOND		1000
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
#define L1			0
#define L2			1
#define L3			2
#define L4			3

#define T1			0
#define T2			1
#define T3			2
#define T4			3

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

/*
 ***************************************************************************
 * sysfs_gpio_handler - handle GPIO operations
 ***************************************************************************
 */
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

/*
 ***************************************************************************
 * Define the function to be called when ctrl-c (SIGINT)
 * signal is sent to process
 ***************************************************************************
 */
void signal_ctrlc_handler(int sig_num)
{
    int i;

    /* Inform user */
    printf("\nExit via Ctrl-C\n\n");

    /* Unexport all selected gpios */
    for (i=0; i<MAX_GPIO; i++) {
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_led[i], NULL);
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_btn[i], NULL);
    }

    /* Terminate program */
    exit(sig_num);
}

/*
 ***************************************************************************
 * Define the function to be called when the process terminate. (SIGTERM)
 ***************************************************************************
 */
void signal_terminate_handler(int sig_num)
{
    int i;

    /* Inform user */
    printf("\nExit via SIGTERM\n\n");

    /* Unexport all selected gpios */
    for (i=0; i<MAX_GPIO; i++) {
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_led[i], NULL);
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_btn[i], NULL);
    }

    /* Terminate program */
    exit(sig_num);
}

/*
 ***************************************************************************
 * main
 ***************************************************************************
 */

int main(int argc, char **argv)
{
    sigset_t set;
    char    button[3];
    uint8_t i;

    /* Register SIGINT CTRL-C signal handler */
    signal(SIGINT, signal_ctrlc_handler);

    /* Register SIGTERM signal handler */
    signal(SIGTERM, signal_terminate_handler);

    /* Initializes the signalmask to empty */
    sigemptyset(&set);

    /* Set the signal mask for the signal handler */
    sigaddset(&set, SIGALRM );

    /* Setup gpio sysfs for the LEDs [L1..L4] and Buttons [T1..T4] */
    for (i=0; i<MAX_GPIO; i++) {
        sysfs_gpio_handler(GPIO_SYSFS_EXPORT, gpio_led[i], NULL);
        sysfs_gpio_handler(GPIO_SYSFS_SET_DIRECTION, gpio_led[i], OUT);
        sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[i], OFF);
        sysfs_gpio_handler(GPIO_SYSFS_EXPORT, gpio_btn[i], NULL);
        sysfs_gpio_handler(GPIO_SYSFS_SET_DIRECTION, gpio_btn[i], IN);
    }

    while (1) {
        // Read values
        for(i = 0; i < 4; i++){
            sysfs_gpio_handler(GPIO_SYSFS_GET_VALUE, gpio_btn[i], &button[i]);
        }

        // Set values
        for(i = 0; i < 4; i++){
            if(button[i] == PRESSED){
                sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[i], ON);
            } else {
                sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[i], OFF);
            }
        } 
    }  

    /* Set default signal handling */
    signal(SIGINT, SIG_DFL);

    /* Unexport all selected gpios */
    for (i=0; i<MAX_GPIO; i++) {
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_led[i], NULL);
        sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_btn[i], NULL);

    }

    return EXIT_SUCCESS;
}

