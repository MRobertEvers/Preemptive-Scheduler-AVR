/*
 * megos_system.h
 *
 * Created: 1/12/2018 5:02:56 PM
 *  Author: Matthew
 */ 


#ifndef MEGOS_SYSTEM_H_
#define MEGOS_SYSTEM_H_

#define CLOCK_FREQUENCY 20000000
#define CLOCK_TICKS_PER_MS CLOCK_FREQUENCY/1000

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

unsigned int megos_millis_get_ticks(unsigned int aiMilliseconds);

#endif /* MEGOS_SYSTEM_H_ */