#include "coSensor.h"
#include <math.h>

float adcCoCalc(int value)
{
	return log((94.37751004f/value)-0.06084945844f)*1900/(-1.8971f)+100;
}
