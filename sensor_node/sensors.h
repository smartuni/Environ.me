#ifndef SENSORS_H
#define SENSORS_H

int init_sensors(void);
int get_temperature(void);
int get_humidity(void);
long get_illuminance(void);

#endif /* SENSORS_H */
