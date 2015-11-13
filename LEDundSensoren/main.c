/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       test app phytec board
 *
 * @author      Kai
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "xtimer.h"
#include "thread.h"
#include "hdc1000.h"
//#include "periph/spi.h"
#include "periph/gpio.h"
#include "ledcontrol.h"


char t2_stack[THREAD_STACKSIZE_MAIN];
char t3_stack[THREAD_STACKSIZE_MAIN];
char t4_stack[THREAD_STACKSIZE_MAIN];

unsigned int colorArray[30];

void *second_thread(void *arg)			//sendet 20fps ans led band
{
    (void) arg;
    
    while(1)
    {
    	sendeArray(colorArray);
    	xtimer_usleep(50000);
    }
    
    return NULL;
}


void *third_thread(void *arg)			//verändert das led-color-array
{
    (void) arg;
    int index = 0;
    xtimer_sleep(1);
    
    while(1)
	{
		/*
		if(index>0)
		{
			writeLed(colorArray,index-1,0,0,0);
		}*/
		resetArray(colorArray);
		writeLed(colorArray,index,0,10,0);
		writeLed(colorArray,index+1,0,50,0);
		writeLed(colorArray,index+2,0,10,0);
		index++;
		if(index==28)
		{
			resetArray(colorArray);
			index=0;
		}
		
		xtimer_usleep(50000);

	}
    return NULL;
}

void *fourth_thread(void *arg)			//init hdc1000 und temp auslesen
{
    	(void) arg;
   	hdc1000_t devHdc;


	uint16_t rawtemp, rawhum;
	int temp, hum;
	
    	if (hdc1000_init(&devHdc, 0, HDC1000_I2C_ADDRESS) == 0) {
        	puts("HDC1000 OK\n");
	}
	else {
		puts("HDC1000 Failed");
		return NULL;
	}

	
    	while(1)
	{
		if (hdc1000_startmeasure(&devHdc)) {
            		puts("HDC1000 Start measure failed.");
            		return NULL;
		}
		xtimer_usleep(HDC1000_CONVERSION_TIME);	//26000us

		hdc1000_read(&devHdc, &rawtemp, &rawhum);
		printf("HDC1000 Raw data T: %5i   RH: %5i\n", rawtemp, rawhum);

		hdc1000_convert(rawtemp, rawhum,  &temp, &hum);
		printf("HDC1000 Data T: %d   RH: %d\n\n", temp, hum);
		xtimer_sleep(1);

	}
    return NULL;
}


int main(void)
{
	int retVal = 0;
	
	// DIO 11, auf dem board, neben der rgb-led, 6. von oben
	retVal = gpio_init(1, GPIO_DIR_OUT, GPIO_PULLDOWN);
	
	if(retVal!=0)
	{
		puts("GPIO_init pin Failed");
		return -1;
	}
	
	resetArray(colorArray);
	
	(void) thread_create(
            t2_stack, sizeof(t2_stack),
            THREAD_PRIORITY_MAIN - 1, CREATE_WOUT_YIELD | CREATE_STACKTEST,
            second_thread, NULL, "nr2");
            
        
        (void) thread_create(
            t3_stack, sizeof(t3_stack),
            THREAD_PRIORITY_MAIN - 1, CREATE_WOUT_YIELD | CREATE_STACKTEST,
            third_thread, NULL, "nr3");
   
    	
    	(void) thread_create(
            t4_stack, sizeof(t4_stack),
            THREAD_PRIORITY_MAIN - 1, CREATE_WOUT_YIELD | CREATE_STACKTEST,
            fourth_thread, NULL, "nr4");
    	
    	while(1)
    	{
    		xtimer_sleep(1);
    	}
    	return 0;
}
