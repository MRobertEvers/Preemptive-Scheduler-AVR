/*
 * megos_system.h
 *
 * Created: 1/12/2018 5:02:56 PM
 *  Author: Matthew
 */ 


#ifndef MEGOS_SYSTEM_H_
#define MEGOS_SYSTEM_H_

#define CLOCK_FREQUENCY 20000000
#define CLOCK_TICKS_PER_MS 20000
#define CLOCK_TICKS_PER_BURST 1000
#define CLOCK_TICKS_PER_MICROS 20

/**
* megos_sched_timer_set(int)
* 
* Calculates and sets the settings that timer 0 needs to  interrupt
* once per the given interval. Enables global interrupts.
*
* @Param aiBursts: (1/20)*milliseconds length of interval. I.e. 20 is 1 millisecond.
*
* Notes: See Atmega328P manual 19.9.x
*        This function performs all actions necessary to start receiving
*        interrupts from timer 0. (Other than define the interrupt). This
*        function also sets the prescaling value.
*        Additionally, with the current implementation, the maximum timer
*        cycle time is about 12 milliseconds (240 'bursts').
*        This is because the prescaler divides by about 1000 at most, meaning
*        1000*256 gives the longest delay time. (1024 is the actual number)
*/
void megos_sched_timer_set(unsigned int aiBursts);

/**
* megos_millis_get_ticks(unsigned int)
*
* Calculates the number of ticks in the input milliseconds.
*
* @Param aiMilliseconds: Interval to delay. Time scale not known.
*/
unsigned int megos_millis_get_ticks(unsigned int aiMilliseconds);

/**
* inline megos_delay_ns(unsigned int)
*
* More granular delay. Despite 'ns' this does not actually achieve
* that resolution.
*
* @Param microseconds: Interval to delay. Time scale not known.
*/
static void inline megos_delay_mus(unsigned int microseconds)
{
   volatile unsigned long tick_s = microseconds*CLOCK_TICKS_PER_MICROS/19;

   // I looked at the dissasembly of this while loop. 
   // Its 19 instructrions per loop.
   // That means we should divide by 19 for each.
   while(tick_s-- > 0);
}

#endif /* MEGOS_SYSTEM_H_ */