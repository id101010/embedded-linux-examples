/*
 ***************************************************************************
 * \brief   Embedded-Linux (BTE5446)
 *          Beagle Bone Black Driver Exercise -- Single LED output
 *
 * \file    led_driver_quad.c
 * \version 1.0
 * \date    08.01.2016
 * \author  Aaron Schmocker
 *
 *   The driver supports communication by ascii strings.
 *   echo "on"  > /dev/led0..3  to set LED1 on
 *   echo "off" > /dev/led0..3  to set LED1 off
 *   cat /dev/led0..3              to get the status of LED1
 *
 *   This driver allows you to flash LED1..4 on the BBB-BFH-Cape
 *
 *   General purpose I/O (GPIO) is used to drive the LED.
 *   Current GPIO assignments for the LED:
 *	 LED1	GPIO# is 61;  Pin P8-26, GPIO1_29
 *   LED2   GPIO# is 44;
 *   LED3   GPIO# is 68;
 *   LED4   GPIO# is 67;
 *
 *
 *   Insert the driver with: modprobe led_driver_quad
 *   Remove the driver with: modprobe -r led_driver_quad
 *
 * \remark  Last Modifications:
 * \remark  V1.0, SCHMA5, 08.01.2016   Initial release
 ***************************************************************************
 *
 * Copyright (C) 2015 Martin Aebersold, Bern University of Applied Scinces
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/* Usual module includes */
#include <linux/types.h>
#include <linux/version.h>	/* Distribution and kernel version */
#include <linux/module.h>	/* Kernel module stuff and macros */
#include <linux/kernel.h>	/* Kernel prog stuff : kprintk, types, ... */
#include <linux/init.h>		/* Required for module_{init,exit}  */
#include <linux/device.h>	/* The Linux device model */
#include <linux/cdev.h>		/* Char device structure and helper */
#include <linux/fs.h>		/* Several file_operations structure */

/* Other includes used */
#include <linux/errno.h>	/* Errnos defs : EPERM, ENOENT, ...*/
#include <linux/fs.h>		/* fs ops */
#include <asm/uaccess.h>	/* copy_to_user, copy_from_usr, ... */
#include <asm/gpio.h>

/*****************************************************************************/
/* Macros and Constants							                             */
/*****************************************************************************/

#define	DEV_NODE_NAME		"led%d"

#define MOD_LICENSE 		"GPL"
#define MOD_AUTHOR 		    "Aaron Schmocker <aaron@duckpond.ch>"
#define MOD_DESCRIPTION 	"Your first Linux driver"
#define MODULE_NAME 		"led_driver_quad"

#define LED_DRIVER_VER		"1.0"

#define FIRTS_MINOR_NR		0
#define HOW_MANY_MINORS		4

#define MAX_MSG_SIZE		32

#define LED_1			    61
#define LED_2			    44
#define LED_3			    68
#define LED_4			    67

#define ON			        0
#define OFF			        1

/*****************************************************************************/
/* Variables etc. 							                                 */
/*****************************************************************************/

uint16_t leds[HOW_MANY_MINORS]  = { LED_1, LED_2, LED_3, LED_4 };

static dev_t         first_dev;	  	/* First device number	*/
static struct cdev   char_dev; 	  	/* The character device	*/
static struct class *dev_class;   	/* The device class	*/

/****************************************************************************/
/* File open								                                */
/****************************************************************************/

static int led_open(struct inode *inode, struct file *file_ptr)
{
    int major, minor;

    major = imajor(inode);
    minor = iminor(inode);
    printk(KERN_INFO "Opening device node at major %d minor %d\n", major, minor);
    return 0;
}

/****************************************************************************/
/* File close operations						                            */
/****************************************************************************/

static int led_close(struct inode *inode, struct file *file_ptr)
{
    int major, minor;

    /* Get major and minor number */
    major = imajor(inode);
    minor = iminor(inode);
    printk(KERN_INFO "Closing device node at major %d minor %d\n", major, minor);
    return 0;
}

/****************************************************************************/
/* File read operations							                            */
/****************************************************************************/

static ssize_t led_read(struct file *file_ptr, char __user *user_buffer, size_t count, loff_t *f_pos)
{
    size_t len = count, status_length;
    ssize_t retval = 0;
    unsigned long ret = 0;
    int major, minor;
    char led_value;
    char *onOff;

    /* Get major and minor number */
    major = MAJOR(file_ptr->f_path.dentry->d_inode->i_rdev);
    minor = MINOR(file_ptr->f_path.dentry->d_inode->i_rdev);

    /* Get led status */
    led_value = gpio_get_value(leds[minor]);
    if (!led_value) {
        onOff = "\non\n";
    } else {
        onOff = "\noff\n";
    }
    status_length = strlen(onOff);

    /* Check and adjust limits */
    if (*f_pos >= status_length) {
        return 0;
    }
    if (*f_pos + count > status_length) {
        len = status_length - *f_pos;
    }

    /* Copy message to user */
    ret = copy_to_user(user_buffer, onOff, len);
    *f_pos += len - ret;
    retval = len - ret;
    return retval;
}

/****************************************************************************/
/* File write operations						                            */
/****************************************************************************/

static ssize_t led_write(struct file *file_ptr, const char __user *user_buffer, size_t size, loff_t *pos)
{
    int major, minor;
    char msgBuffer[MAX_MSG_SIZE];
    char led_value;

    /* Get major and minor number */
    major = MAJOR(file_ptr->f_path.dentry->d_inode->i_rdev);
    minor = MINOR(file_ptr->f_path.dentry->d_inode->i_rdev);

    /* Check size of user message */
    if (size > MAX_MSG_SIZE) {
        return -EINVAL;
    }

    if (copy_from_user(msgBuffer, user_buffer, size) != 0) {
        return -EFAULT;
    }

    /* Get command for the leds from the user  */
    if (strncmp(msgBuffer, "on", strlen("on")) == 0) {
        led_value = ON;
        printk(KERN_INFO "Info: LED1 on!\n");
    } else if (strncmp(msgBuffer, "off", strlen("off")) == 0) {
        led_value = OFF;
        printk(KERN_INFO "Info: LED1 off!\n");
    } else {
        return size;
    }

    /* Turn led on or off */
    gpio_set_value(leds[minor], led_value);
    return size ;
}

/****************************************************************************/
/* File operations implemented by this driver				                */
/****************************************************************************/

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_close,
    .read = led_read,
    .write = led_write
};

/****************************************************************************/
/* Module initialization (Constructor)					                    */
/****************************************************************************/

static int __init led_init(void)
{
    int ret,i;
    struct device *dev_ret;

    /* Inform user */
    printk(KERN_INFO "\nInfo: led_driver_single registered!\n");
    printk(KERN_INFO "Info: %s V%s\n\n", MODULE_NAME, LED_DRIVER_VER);

    /* Try to request the gpio for L1 on the BBB-BFH-CAPE */
    for (i=0; i<HOW_MANY_MINORS; i++) {
        if ((ret = gpio_request(leds[i], "LED1")) < 0) {
            return ret;
        }
    }

    /* Set the gpio of L1 on the BBB-BFH-CAPE as output */
    for (i=0; i<HOW_MANY_MINORS; i++) {
        if ((ret = gpio_direction_output(leds[i], OFF)) < 0) {
            return ret;
        }
    }

    /* Allocates a range of char device numbers */
    if ((ret = alloc_chrdev_region(&first_dev, FIRTS_MINOR_NR, HOW_MANY_MINORS, MODULE_NAME)) < 0) {
        return ret;
    }

    /* Create a struct class pointer to be used in device_create() */
    if (IS_ERR(dev_class = class_create(THIS_MODULE, MODULE_NAME))) {
        unregister_chrdev_region(first_dev, HOW_MANY_MINORS);
        return PTR_ERR(dev_class);
    }

    /* Create device in sysfs and register it to the specified class */
    for (i=0; i<HOW_MANY_MINORS; i++) {
        if (IS_ERR(dev_ret = device_create(dev_class, NULL, MKDEV(MAJOR(first_dev), MINOR(first_dev) + i), NULL, DEV_NODE_NAME, i))) {
            class_destroy(dev_class);
            unregister_chrdev_region(first_dev, HOW_MANY_MINORS);
            return PTR_ERR(dev_ret);
        }
    }
    /* Initializes cdev, remembering fops, making it ready to add to the system */
    cdev_init(&char_dev, &led_fops);

    /* Adds the device represented by char_dev to the system */
    if ((ret = cdev_add(&char_dev, first_dev, HOW_MANY_MINORS)) < 0) {
        device_destroy(dev_class, first_dev);
        class_destroy(dev_class);
        unregister_chrdev_region(first_dev, HOW_MANY_MINORS);
        return ret;
    }
    return 0;
}

/****************************************************************************/
/* Module cleanup (Destructor)						                        */
/****************************************************************************/

static void __exit led_exit(void)
{
    int i;

    /* Remove char_dev from the system */
    cdev_del(&char_dev);

    /* Unregisters and cleans up devices created with a call to device_create */
    for (i=0; i<HOW_MANY_MINORS; i++) {
        device_destroy(dev_class, MKDEV(MAJOR(first_dev), MINOR(first_dev) + i));
    }

    /* Destroy an existing class */
    class_destroy(dev_class);

    /* Unregister a range of device numbers */
    unregister_chrdev_region(first_dev, HOW_MANY_MINORS);

    /* Release previously requested gpios */
    for (i=0; i<HOW_MANY_MINORS; i++) {
        gpio_free(leds[i]);
    }

    /* Inform user */
    printk(KERN_INFO "\nInfo: led_driver_quad unregistered!\n\n");
}

/************************************************************************/
/* Driver init and exit entry points					                */
/************************************************************************/

module_init(led_init);
module_exit(led_exit);

/************************************************************************/
/* Module info								                            */
/************************************************************************/

MODULE_LICENSE(MOD_LICENSE);
MODULE_DESCRIPTION(MOD_DESCRIPTION);
MODULE_AUTHOR(MOD_AUTHOR);

