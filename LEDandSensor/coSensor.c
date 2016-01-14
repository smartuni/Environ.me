/*
implementation of the function to calculate the corresponding gas sensors ppm value to given adc value

author Kai
*/

#include "coSensor.h"
#include <math.h>

float adcCoCalc(int value)	//calculates given int value from adc into real ppm value corresponding to co sensor MQ-7
{
	return log((94.37751004f/value)-0.06084945844f)*1900/(-1.8971f)+100;
}
