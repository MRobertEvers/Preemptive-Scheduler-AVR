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

static int test_int = 0;
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
      megos_task_sleep(500);
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
      megos_task_sleep(500);
   }
}

int main(void)
{
   megos_init();
   int i = megos_new_task(&test_task, SCHEDULER_DEFAULT_TASK_SIZE);
   megos_task_start(i);

   i = megos_new_task(&test_task_two, SCHEDULER_DEFAULT_TASK_SIZE);
   megos_task_start(i);
   /* Replace with your application code */
   while (1)
   {
   }
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
   // The program counter is on the stack at this point. If we were to leave this context,
   // then upon return, the exit procedure for leaving an ISR is to pop the return address
   // off the stack. Since the return address for an ISR will be the address that was
   // interrupted, pointing to the stack pointer of this context will cause that address
   // to be returned to.
   // TODO: Explain this better.
   megos_schedule(1);
}
