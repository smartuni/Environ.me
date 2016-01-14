/*
This file is the main.c to be flashed onto an attiny45 for sending the right bit pattern to the WS2812 led stripe

author: Kai
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#define nop() asm volatile("nop")

int main(void)
{
  
  OSCCAL = 96;  //higher = faster figure 22-42 data sheet
  CLKPR = 0x80;  //system clock to 8mhz no prescaler
  CLKPR = 0x00;  //these 2 CLKPR instrs must be run together
  
  DDRB |= (1<<PB0);  //PB0 output
  
  
  GIMSK |= (1<<PCIE);   //enable Pinchange interrupts
  PCMSK |= (1<<PCINT3);   //enable Pinchange interrupt on PB3
  sei();    //enable global interrupts
  
 while(1)   //endless loop
 {
   nop();
 } 
  
  return 0;
}

ISR(PCINT3_vect)  //Pinchange interrupt vector
{
  if(PINB & (1<<PINB4))
   {
     sendOne();
   } else
   {
     sendZero();
   }
}


void sendOne(void)  //sends 940ns High and at least 235ns Low to transmit 1-Bit
{
  PORTB |= (1<<PB0);
  nop();
  nop();
  nop();
  nop();
  PORTB &= ~(1<<PB0);
  nop();
}

void sendZero(void) //sends 235ns High and at least 700ns Low to transmit 0-Bit
{
  PORTB |= (1<<PB0);
  nop();
  PORTB &= ~(1<<PB0);
  nop();
  nop();
  nop();
}
