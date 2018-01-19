/*
* ATmega328PMegOS.c
*
* Created: 1/11/2018 9:40:51 PM
* Author : Matthew
*/

#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include "megos.h"
#include "megos_8led_driver.h"
#include "megos_scheduler.h"
#include "megos_settings.h"
#include "megos_contexts.h"

int main(void)
{
   megos_init();
   megos_8led_init();
   while(1)
   {
      for(int i = 0; i < 8; i++)
      {
         megos_8led_display_set_row(i, i);
      }
      megos_8led_draw_refresh();
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
   megos_interrupt_schedule();
}
