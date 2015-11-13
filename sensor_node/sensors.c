#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "xtimer.h"
#include "hdc1000.h"

static hdc1000_t devHdc;

static void hdc1000_measure(int *temp, int *hum);

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
    int temp = -100000;
    int hum = -1;
    hdc1000_measure(&temp, &hum);
    return temp;
}

int get_humidity(void) {
    int temp = -100000;
    int hum = -1;
    hdc1000_measure(&temp, &hum);
    return hum;
}

void hdc1000_measure(int *temp, int *hum) {
    uint16_t rawtemp;
    uint16_t rawhum;
    if (hdc1000_startmeasure(&devHdc)) {
	puts("[sensors] HDC1000 starting measure failed");
	return;
    }
    xtimer_usleep(HDC1000_CONVERSION_TIME); //26000us
    hdc1000_read(&devHdc, &rawtemp, &rawhum);
    hdc1000_convert(rawtemp, rawhum,  temp, hum);
    printf("[sensors] INFO: HDC1000 Data T: %d   RH: %d\n", *temp, *hum);
}
