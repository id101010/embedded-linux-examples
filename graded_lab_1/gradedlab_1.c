/*
 ***************************************************************************
 * \brief   Embedded-Linux (BTE5446)
 *	    Linux Sysfs GPIO Exercise 1.0, Template
 *          Use this template for your apps and adjust it accordingly.
 * \file    appSysfsTemplate.c
 * \version 1.0
 * \date    25.10.2013
 * \author  Aaron Schmocker
 *
 * \remark  Last Modifications:
 * \remark  V1.0, AOM1, 25.10.2013   Initial release
 * \remark  V1.1, AOM1, 20.11.2015   Added POSIX Timer Handling
 * \remark  V1.2, SCHMA5, 30.11.2015 Implemented moving light
 ***************************************************************************
 *
 * Copyright (C) 2015 Aaron Schmocker, Bern University of Applied Scinces
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
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#undef DEBUG

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

static bool dir;
static char button[3];
static int state;

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
 * kill all leds
 ***************************************************************************
 */
void kill_all_leds(void)
{
    int i = 0;
    for(i = T1; i <= T4; i++) {
        sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[i], OFF);
    }
}

/*
 ***************************************************************************
 * update buttons
 ***************************************************************************
 */
void update_buttons()
{
    int i = 0;
    char new_button[4]  = {0,0,0,0};
    char edge_button[4] = {0,0,0,0};

    // Read button values
    for(i = 0; i < 4; i++) {
        sysfs_gpio_handler(GPIO_SYSFS_GET_VALUE, gpio_btn[i], &new_button[i]);
    }

    // Edge detection
    for(i = 0; i < 4; i++) {
        edge_button[i] = ((new_button[i] == PRESSED) && (button[i] != PRESSED));
        button[i] = new_button[i];
    }

    // Button T1: End Programm
    if(edge_button[T1]) {
        signal(SIGINT, SIG_DFL);

        for (i=0; i<MAX_GPIO; i++) {
            sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_led[i], NULL);
            sysfs_gpio_handler(GPIO_SYSFS_UNEXPORT, gpio_btn[i], NULL);
        }
        
        exit(EXIT_SUCCESS);
    }

    // Button T2: Double frequency
    if(edge_button[T2]) {
        state++;
        if(state > 5) {
            state = 5;
        }
    }

    // Button T3: Divide frequency by two
    if(edge_button[T3]) {
        state--;
        if(state < 1) {
            state = 1;
        }
    }

    // Button T4: Change direction of the moving light effect
    if(edge_button[T4]) {
        dir = !dir;
    }
}

/*
 ***************************************************************************
 * sleep for less than a second
 ***************************************************************************
 */
int nsleep(long miliseconds)
{
    struct timespec req, rem;

    if(miliseconds > 999) {
        req.tv_sec = (int)(miliseconds / 1000);
        req.tv_nsec = (miliseconds - ((long)req.tv_sec * 1000)) * 1000000;
    } else {
        req.tv_sec = 0;
        req.tv_nsec = miliseconds * 1000000;
    }

    return nanosleep(&req , &rem);
}

/*
 ***************************************************************************
 * sleep for less than a second and do something while waiting
 ***************************************************************************
 */
void advanced_sleep(long base_ms, int multiplicator)
{
    int i = 0;

    for(i = 0; i < multiplicator; i++) {
        update_buttons();
        nsleep(base_ms);
    }
}

/*
 ***************************************************************************
 * Displays a moving light effect
 ***************************************************************************
 */
void moving_light(long miliseconds, bool direction)
{
    int i = 0;

    if(!direction) {
        // Moving down
        for(i = 0; i < 4; i++) {
            advanced_sleep(1, miliseconds);
            kill_all_leds();
            sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[i], ON);
        }
    } else {
        // Moving up
        for(i = 3; i >= 0; i--) {
            advanced_sleep(1, miliseconds);
            kill_all_leds();
            sysfs_gpio_handler(GPIO_SYSFS_SET_VALUE, gpio_led[i], ON);
        }
    }
}

/*
 ***************************************************************************
 * main method
 ***************************************************************************
 */
int main(int argc, char **argv)
{
    sigset_t set;
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

    state   = 1;            // initial state
    dir     = false;        // direction up

    while (1) {

        // Main state machine
        switch(state) {
        case 1:
            moving_light(1000, dir);
            break;
        case 2:
            moving_light(500, dir);
            break;
        case 3:
            moving_light(250, dir);
            break;
        case 4:
            moving_light(125, dir);
            break;
        case 5:
            moving_light(62, dir);
            break;
        default:
            printf("state error \n");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

