/*
 ***************************************************************************
 * \brief   Embedded Linux poti_value
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
 * \file    poti_value.c
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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <math.h>

#include <sys/stat.h>

/* String to access the ADC4 via sysfs */
#define AIN4_DEV	"/sys/bus/iio/devices/iio\:device0/in_voltage4_raw"

/* Delay value for one second in us */
#define ONE_SECOND	1000000

/* Define some useful constants */
#define BUFFER_SIZE		16
#define ADC_BIT_RES		12
#define V_REF			1.8
#define N			pow(2,12)

/* Vars */
int   adc_fd;
int   charRead;
float aValue;
char  adcBuffer[BUFFER_SIZE];

/* Singnal handler for CTRL-C */
void sigint_handler(int sig);

/*
 ***************************************************************************
 * Define the function to be called when ctrl-c (SIGINT)
 * signal is sent to process
 ***************************************************************************
 */

void signal_callback_handler(int signum)
{
    /* Inform user */
    printf("\nExit via Ctrl-C\n\n");

    /* Close the adc file */
    close(adc_fd);

    /* Terminate program */
    exit(signum);
}

/*
 ***************************************************************************
 * main
 ***************************************************************************
 */

int main(int argc, char *argv[])
{
    /* Register signal and signal handler */
    signal(SIGINT, signal_callback_handler);

    /* Open adc file descriptor */
    adc_fd = open(AIN4_DEV, O_RDONLY);

    /* Check for any errors */
    if (adc_fd == -1) {
        perror("Error: cannot open adc device!\n");
        return -1;
    }

    /* Do until CTRL-C */
    while (1) {
        /* Get ADC value as string */
        charRead = read(adc_fd, adcBuffer, sizeof(adcBuffer));
        if (charRead != -1) {
            /* Terminate string */
            adcBuffer[charRead] = '\0';

            /* Calculate input voltage */
            aValue = (float) atoi(adcBuffer);
            aValue = (V_REF * aValue) / (N-1);
            sprintf(adcBuffer, "%f", aValue);

            /* Write input voltage to console */
            printf("AIN4: %sV\n", adcBuffer);
            lseek(adc_fd, 0, 0);
        }
        sleep(1);
    }
    /* Clean up and exit */
    close(adc_fd);
    return 0;
}


