/*
* ATmega328PMegOS.c
*
* Created: 1/11/2018 9:40:51 PM
* Author : Matthew
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "megos.h"
#include "megos_scheduler.h"
#include "megos_settings.h"

void test_task(void)
{
   DDRD |= 0x7C;
   unsigned char bit = 1;
   while(1)
   {
      PORTD &= !(bit << PORTD2);
      PORTD |= (bit << PORTD2);
      bit = bit << 1;
      if(bit > (1 << 4 << PORTD2))
      {
         bit = 1;
      }
      megos_task_sleep(250);
   }
}

void test_task_two(void)
{
   DDRB |= 0x7C;
   unsigned char bit = 1;
   while(1)
   {
      PORTB &= !(bit << PORTB2);
      PORTB |= (bit << PORTB2);
      bit = bit << 1;
      if(bit > (1 << 4 << PORTB2))
      {
         bit = 1;
      }
      megos_task_sleep(250);
   }
}

int main(void)
{
   megos_init();
   int i = megos_new_task(&test_task, SCHEDULER_DEFAULT_TASK_SIZE);
   megos_task_start(i);

   i = megos_new_task(&test_task_two, SCHEDULER_DEFAULT_TASK_SIZE);
   megos_task_start(i);

   while (1);
}

// Create the ISR for the scheduler.
// The interrupt rate must be set elsewhere.
// According to the avr-gcc manual, the name of the interrupt must take the
// form of <interrupt_source>_vect or SIG_<interrupt_source>. SIG_ is soon
// to be deprecated, so use _vect.
// The interrupt source we want is one of the timers. We will choose timer 0.
// See page 82 of Atmega328P for interrupt source.
// http://www.microchip.com/webdoc/avrlibcreferencemanual/group__avr__interrupts.html
ISR(TIMER0_COMPA_vect)
{
   // The program counter for this task is put on the stack by this function call.
   // We save the context on the stack above the program counter, then we move
   // the stack pointer to the next task. Upon return to this task, the context
   // is popped of the previously saved stack. Once that is done, the program
   // counter is popped off the stack as part of the ret command and the program
   // counter is returned to where it left off on this task -- some function
   // inside this ISR.
   // Since the return address for an ISR will be the address that was
   // interrupted, the program flow will return to task operations when this
   // ISR returns.
   megos_schedule(1);
}
