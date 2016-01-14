#ifndef LEDCONTROL_H
#define LEDCONTROL_H


void writeLed(unsigned int array[], int led, unsigned char green, unsigned char red, unsigned char blue);
void resetArray(unsigned int array[]);
void sendZero(void);
void sendOne(void);
void sendArray(unsigned int array[]);

#endif
