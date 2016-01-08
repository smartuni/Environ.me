#ifndef LEDCONTROL_H
#define LEDCONTROL_H


void sendHigh(int dev);
void sendLow(int dev);
void writeLed(unsigned int array[], int led, unsigned char green, unsigned char red, unsigned char blue);
void resetArray(unsigned int array[]);
void sendArray(unsigned int array[], int dev);
void sendeNull(void);
void sendeEins(void);
void sendeArray(unsigned int array[]);

#endif
