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
   while(1)
   {
      DDRB |= (test_int << PORTB5);
      PORTB &= !(test_int << PORTB5);
      PORTB |= (test_int << PORTB5);
      test_int = !test_int;
      megos_task_sleep(1000);
   }
}

int main(void)
{
   megos_init();
   megos_new_task(&test_task, SCHEDULER_DEFAULT_TASK_SIZE);
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
