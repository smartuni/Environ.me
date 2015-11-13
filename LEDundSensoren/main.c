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
#include "hdc1000.h"
#include "periph/spi.h"
#include "periph/gpio.h"
#include "ledcontrol.h"



int main(void)
{

        hdc1000_t devHdc;


        uint16_t rawtemp, rawhum;
        int temp, hum;

       

        int retVal=0;
       
        unsigned int colorArray[30];
        resetArray(colorArray);
//      writeLed(colorArray,0,0,127,0);
       


        puts("\nHello World!");

        printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
        printf("This board features a(n) %s MCU.\n\n", RIOT_MCU);
       

       
        if (hdc1000_init(&devHdc, 0, HDC1000_I2C_ADDRESS) == 0) {
        puts("HDC1000 OK\n");
        }
        else {
                puts("HDC1000 Failed");
                return -1;
        }
       
        // DIO 11, auf dem board, neben der rgb-led, 6. von oben
        retVal=gpio_init(1, GPIO_DIR_OUT, GPIO_PULLDOWN);
        if(retVal!=0)
        {
                puts("GPIO_init pin Failed");
                return -1;
        }
       
        sendeArray(colorArray);

        int increment=0;
        int flag=0;
        while (1)
        {
        //      increment=0;
        //      sendeArray(colorArray);
                xtimer_usleep(10000);
                if(flag==1)
                {
                        if(increment>0)
                        {
                                writeLed(colorArray,increment-1,0,0,0);
                        }
                       
                        writeLed(colorArray,increment,50,0,0);
                        writeLed(colorArray,increment+1,0,50,0);
                        writeLed(colorArray,increment+2,0,0,50);
                        sendeArray(colorArray);
                        increment++;
                }
                else
                {
                        xtimer_usleep(200000);
                }
               
               
                if(increment>27)
                {
                        increment=0;
                        resetArray(colorArray);
                        sendeArray(colorArray);
                }


               
         if (hdc1000_startmeasure(&devHdc)) {
                         puts("HDC1000 Start measure failed.");
                         return -1;
                }
                xtimer_usleep(HDC1000_CONVERSION_TIME); //26000us

                hdc1000_read(&devHdc, &rawtemp, &rawhum);
                printf("HDC1000 Raw data T: %5i   RH: %5i\n", rawtemp, rawhum);

                hdc1000_convert(rawtemp, rawhum,  &temp, &hum);
                printf("HDC1000 Data T: %d   RH: %d\n\n", temp, hum);

                if(temp>2590)
                {
                        flag=1;
                }
                if(temp<2590)
                {
                        flag=0;
                        increment=0;
                        resetArray(colorArray);
                        sendeArray(colorArray);
                }
//              xtimer_usleep(100000);
               

        }

       
        return 0;
}
