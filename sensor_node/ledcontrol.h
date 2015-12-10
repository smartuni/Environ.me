#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#define LED_MODE_OFF   (0)
#define LED_MODE_LEFT  (1)
#define LED_MODE_RIGHT (2)

void start_led_control(void);
void set_led(uint32_t mode);

#endif /* SENSORS_H */
