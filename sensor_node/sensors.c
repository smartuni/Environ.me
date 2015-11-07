#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "xtimer.h"
#include "hdc1000.h"

static hdc1000_t devHdc;
static uint16_t rawtemp;
static uint16_t rawhum;
static int temp;
static int hum;

int init_sensors(void) {
    // temperature sensor
    if (hdc1000_init(&devHdc, 0, HDC1000_I2C_ADDRESS) == 0) {
	puts("[sensors] INFO:  HDC1000 initialisation success");
	return 1;
    } else {
	puts("[sensors] ERROR: HDC1000 initialisation failed");
	return 0;
    }
}

int get_temperature(void) {
    if (hdc1000_startmeasure(&devHdc)) {
	puts("[sensors] HDC1000 starting measure failed");
	return -1;
    }
    xtimer_usleep(HDC1000_CONVERSION_TIME); //26000us
    hdc1000_read(&devHdc, &rawtemp, &rawhum);
    hdc1000_convert(rawtemp, rawhum,  &temp, &hum);
    printf("[sensors] INFO: HDC1000 Data T: %d   RH: %d\n", temp, hum);
    return temp;
}
