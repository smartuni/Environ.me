/**
 * @ingroup     LED-Control
 * @{
 *
 * @file
 * @brief       test app phytec board
 *
 * @author      Kai and Jens
 *
 * @}
 */

#include "ledcontrol.h"
#include <string.h>
#include <stdlib.h>
#include "periph/spi.h"
#include "periph/gpio.h"
#include "board.h"



void writeLed(unsigned int array[], int led, unsigned char green, unsigned char red, unsigned char blue)	//writes GRB values to a given array position
{
	array[led] = ((green<<16)|(red<<8)|blue);
}

void resetArray(unsigned int array[])	//writes all data in given array to zero
{
	for(int ind=0;ind<30;ind++)
	{
		array[ind]=0;
	}
}


void sendZero(void)	//sends bit pattern needed for detecting one logic Low (350ns High [>0.7V / +-150ns], 800ns Low [<0.3V / +-150ns]) on led pin,
			//timings change frequently with updates of Riot, so the ugly coding style means the only way to keep timings in needed conditions
{
	for(int i=0;i<3;i++)
	{
		LED_R_OFF;
	}

	for(int i=0;i<10;i++)
	{
		LED_R_ON;
	}
}

void sendOne(void)	//sends bit pattern needed for detecting one logic High (700ns High [>0.7V / +-150ns], 600ns Low [<0.3V / +-150ns]) on led pin,
			//timings change frequently with updates of Riot, so the ugly coding style means the only way to keep timings in needed conditions
{
	for(int i=0;i<4;i++)
	{
		LED_R_OFF;
	}
	
	for(int i=0;i<10;i++)
	{
		LED_R_ON;
	}
}

void sendArray(unsigned int array[])		//sends every bit in given data array on led pin
{
	for(int ledPosition=0;ledPosition<30;ledPosition++)
		{
			for(int bitPosition=0;bitPosition<24;bitPosition++)
			{
				if(((array[ledPosition]>>(23-bitPosition))&1)==1)
				{
					sendOne();
				}
				else
				{
					sendZero();
				
				}
			}
		}
}
