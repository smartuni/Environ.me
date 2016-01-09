#ifndef LEDCONTROL_H
#define LEDCONTROL_H


void writeLed(unsigned int array[], int led, unsigned char green, unsigned char red, unsigned char blue);
void resetArray(unsigned int array[]);
void sendeNull(void);
void sendeEins(void);
void sendeArray(unsigned int array[]);

#endif
