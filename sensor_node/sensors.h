/**
 * @brief  Header for the sensors module.
 * 
 * @author Ruediger Bartz
 * @author Timo Gerken
 */

#ifndef SENSORS_H
#define SENSORS_H

int init_sensors(void);
int get_temperature(void);
int get_humidity(void);
long get_illuminance(void);
void get_all(int *temp, int *hum, long *lux);

#endif /* SENSORS_H */
