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
#define CLOCK_TICKS_PER_NS 20

/**
* megos_sched_timer_set(int)
* 
* Calculates and sets the settings that timer 0 needs to  interrupt
* once per the given interval. Enables global interrupts.
*
* @Param aiMilliseconds: Millisecond interval for interrupt to occur.
*
* Notes: See Atmega328P manual 19.9.x
*        This function performs all actions necessary to start receiving
*        interrupts from timer 0. (Other than define the interrupt). This
*        function also sets the prescaling value.
*/
void megos_sched_timer_set(unsigned int aiMilliseconds);

/**
* megos_delay_ns(unsigned int)
*
* More granular delay. Despite 'ns' this does not actually achieve 
* that resolution.
*
* @Param nanosecond: Interval to delay. Time scale not known.
*
*/
void megos_delay_ns(unsigned int nanoseconds);

/**
* megos_millis_get_ticks(unsigned int)
*
* Calculates the number of ticks in the input milliseconds.
*
* @Param aiMilliseconds: Interval to delay. Time scale not known.
*
*/
unsigned int megos_millis_get_ticks(unsigned int aiMilliseconds);

#endif /* MEGOS_SYSTEM_H_ */