/*
* megos_scheduler_timer.c
*
* Created: 1/9/2018 4:13:51 PM
*  Author: Matthew
*/
#include "megos_system.h"
#include <avr/io.h>
#include <avr/interrupt.h>


/*
* sched_timer_enable_CTC(void)
*
* Sets the appropriate registers to enable CTC mode on timer 0.
*
* Notes: See ATmega328P manual 19.9.1.
*        Bit WGM01 of the TCCR0A register is the flag to enable CTC mode.
*/
static void sched_timer_enable_CTC(void);

/*
* sched_timer_set_ticks(unsigned char)
*
* Sets the number of 'ticks' between each interrupt interval
* for timer 0.
*
* @Param aiTicks: Ticks between interrupts.
*
* Notes: See Atmega328P manual 19.9.6.
*        The highest 'tick' value between interrupts is 255.
*        The register that stores the value is OCR0A.
*        Whenever, the timer counter matches the value in OCR0A, the interrupt
*        will flag. If a longer time period is needed than provided by 255 ticks,
*        look into prescaling the timer 'ticks'. See megos_sched_timer_set.
*/
static void sched_timer_set_ticks(unsigned char aiTicks);

/*
* sched_timer_enable(void)
*
* Enables the interrupts for CTC mode (comparing with register OCR0A).
*
* Notes: See Atmega328P manual 19.9.x
*        This flag is the OCIE0A bit of the TIMSK0 register.
*/
static void sched_timer_enable(void);

//////////////////////////////////////////////////////////////////////////
///////////////////////////////Implementation/////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Track what we set the prescale to for convenience.
static unsigned int system_rate;

static void sched_timer_enable_CTC(void)
{
   // Set timer to CTC Mode. See atmel manual 19.9.1
   TCCR0A &= (0xFC);
   TCCR0A |= (0x1 << WGM01);
}

static void sched_timer_set_ticks(unsigned char aiTicks)
{
   // Set the compare value for timer 0 to aiTicks
   // See manual 19.9.6
   OCR0A = aiTicks;
}

static void sched_timer_enable(void)
{
   // See 4b.
   // Sets the interrupt Compare Flag Interrupt Enable Bit.
   TIMSK0 |= (1 << OCIE0A);
}

void megos_sched_timer_set(unsigned int aiMilliseconds)
{
   cli();
   sched_timer_enable();
   sched_timer_enable_CTC();
   
   system_rate = aiMilliseconds;
   int iTicks = CLOCK_TICKS_PER_MS * aiMilliseconds;

   // Clear the Pre-Scaler bits. See atmel manual 19.9.2
   TCCR0B &= (0xF8);
   
   if(iTicks < 256)
   {
      // Set Pre-scaler to 1
      TCCR0B |= (1 << CS00);
   }
   else if( iTicks / 8 < 256 )
   {
      // Set Pre-scaler to 8
      TCCR0B |= (1 << CS01);
      iTicks /= 8;
   }
   else if( iTicks / 64 < 256 )
   {
      // Set Pre-scaler to 64
      TCCR0B |= (1 << CS00);
      TCCR0B |= (1 << CS01);
      iTicks /= 64;
   }
   else if( iTicks / 256 < 256 )
   {
      // Set Pre-scaler to 256
      TCCR0B |= (1 << CS02);
      iTicks /= 256;
   }
   else if( iTicks / 1024 < 256 )
   {
      // Set Pre-scaler to 1024
      TCCR0B |= (1 << CS00);
      TCCR0B |= (1 << CS02);
      iTicks /= 1024;
   }
   else
   {
      // Cant do it.
   }

   sched_timer_set_ticks(iTicks & 0xFF);
   sei();
}

void megos_delay_ns(unsigned int nanoseconds)
{
   volatile unsigned long tick_s = nanoseconds*CLOCK_TICKS_PER_NS/3;
   while(tick_s-- > 0);
}

unsigned int megos_millis_get_ticks(unsigned int aiMilliseconds)
{
   return aiMilliseconds / system_rate;
}