#include "ledcontrol.h"
#include <string.h>
#include <stdlib.h>
#include "periph/spi.h"
#include "periph/gpio.h"

void sendHigh(int dev)
{
	spi_transfer_byte(dev, 0b11110000, NULL);
}

void sendLow(int dev)
{
	spi_transfer_byte(dev, 0b00000000, NULL);
}

void writeLed(unsigned int array[], int led, unsigned char green, unsigned char red, unsigned char blue)
{
	array[led] = ((green<<16)|(red<<8)|blue);
}

void resetArray(unsigned int array[])
{
	for(int ind=0;ind<30;ind++)
	{
		array[ind]=0;
	}
}

void sendArray(unsigned int array[], int dev)
{
	for(int ledPosition=0;ledPosition<30;ledPosition++)
		{
			for(int bitPosition=0;bitPosition<24;bitPosition++)
			{
				if(((array[ledPosition]>>(23-bitPosition))&1)==1)
				{
					sendHigh(dev);
				}
				else
				{
					sendLow(dev);
				
				}
			}
		}
}


void sendeNull(void)
{
	gpio_set(1);
//	gpio_set(1);
	gpio_clear(1);
	gpio_clear(1);
	gpio_clear(1);
}

void sendeEins(void)
{
	gpio_set(1);
	gpio_set(1);
//	gpio_set(1);
	gpio_clear(1);
	gpio_clear(1);
}

void sendeArray(unsigned int array[])
{
	for(int ledPosition=0;ledPosition<30;ledPosition++)
		{
			for(int bitPosition=0;bitPosition<24;bitPosition++)
			{
				if(((array[ledPosition]>>(23-bitPosition))&1)==1)
				{
					sendeEins();
				}
				else
				{
					sendeNull();
				
				}
			}
		}
}
