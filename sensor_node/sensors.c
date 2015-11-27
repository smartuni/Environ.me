#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "xtimer.h"
#include "hdc1000.h"
#include "tcs37727.h"

#ifndef INT32_MIN
#define INT32_MIN -2147483648
#endif

static hdc1000_t hdcDev;
static tcs37727_t tcsDev;

static void hdc1000_measure(int *temp, int *hum);

int init_sensors(void) {
    // temperature/humidity sensor
    if (hdc1000_init(&hdcDev, 0, HDC1000_I2C_ADDRESS) == 0) {
	puts("[sensors] INFO: HDC1000 initialisation success");
    } else {
	puts("[sensors] ERROR: HDC1000 initialisation failed");
	return 0;
    }
    // illuminance sensor
    if (tcs37727_init(&tcsDev, 0, TCS37727_I2C_ADDRESS, TCS37727_ATIME_DEFAULT)
	    == 0) {
        puts("[sensors] INFO: TCS37727 initialisation success");
    } else {
	puts("[sensors] ERROR: TCS37727 initialisation failed");
	return 0;
    }
    return 1;
}

int get_temperature(void) {
    int temp = INT_MIN;
    int hum = -1;
    hdc1000_measure(&temp, &hum);
    return temp;
}

int get_humidity(void) {
    int temp = INT_MIN;
    int hum = -1;
    hdc1000_measure(&temp, &hum);
    return hum;
}

void hdc1000_measure(int *temp, int *hum) {
    uint16_t rawtemp;
    uint16_t rawhum;
    if (hdc1000_startmeasure(&hdcDev)) {
	puts("[sensors] ERROR: HDC1000 starting measure failed");
	return;
    }
    xtimer_usleep(HDC1000_CONVERSION_TIME); //26000us
    hdc1000_read(&hdcDev, &rawtemp, &rawhum);
    hdc1000_convert(rawtemp, rawhum,  temp, hum);
    printf("[sensors] INFO: HDC1000 Data T: %d   RH: %d\n", *temp, *hum);
}

long get_illuminance(void) {
    tcs37727_data_t data;
    if (tcs37727_set_rgbc_active(&tcsDev) != 0) {
	puts("[sensors] ERROR: TCS37727 activation failed");
	return -1;
    }
    xtimer_usleep(2400 + tcsDev.atime_us);
    if (tcs37727_read(&tcsDev, &data) != 0) {
	puts("[sensors] ERROR: TCS37727 reading data failed");
	return -1;
    }
    printf("[sensors] INFO: TCS37727 Data R: %5"PRIu32" G: %5"PRIu32
	   " B: %5"PRIu32" C: %5"PRIu32"\n", data.red, data.green, data.blue,
	   data.clear);
    printf("[sensors] INFO: TCS37727 Data CT : %5"PRIu32" Lux: %6"PRIu32
	   " AGAIN: %2d ATIME %d\n", data.ct, data.lux, tcsDev.again,
	   tcsDev.atime_us);
    if (tcs37727_set_rgbc_standby(&tcsDev) != 0) {
	puts("[sensors] ERROR: TCS37727 deactivation failed");
    }
    return data.lux;
}
