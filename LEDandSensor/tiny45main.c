#include <avr/io.h>

#define nop() asm volatile("nop")

int main(void)
{
  
  OSCCAL = 96;  //higher = faster figure 22-42 data sheet
  CLKPR = 0x80;  //set system clock to 8mhz no prescaler
  CLKPR = 0x00;  //these 2 CLKPR instructions must be run together
  
  DDRB |= (1<<PB0);  //set PB0 output
 
 while(1)
 {
   if(PINB & (1<<PINB3))
   {
     sendZero();
   }
   else if(PINB & (1<<PINB4))
   {
     sendOne();
   }
 } 
  
  return 0;
}

void sendOne(void)
{
  PORTB |= (1<<PB0);
  nop();
  nop();
  nop();
  nop();
  PORTB &= ~(1<<PB0);
}

void sendZero(void)
{
  PORTB |= (1<<PB0);
  nop();
  PORTB &= ~(1<<PB0);
  nop();
  nop();
}
