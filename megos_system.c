/*
 * megos_scheduler_timer.c
 *
 * Created: 1/9/2018 4:13:51 PM
 *  Author: Matthew
 */ 
#include "megos_system.h"
#include <avr/io.h>
#include <avr/interrupt.h>

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

void megos_sched_timer_set(int aiMilliseconds)
{
	cli();
	sched_timer_enable();
	sched_timer_enable_CTC();
	
	long iTicks = CLOCK_TICKS_PER_MS * aiMilliseconds;

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
	
	// Now set the compare value to iMinResolution
	sched_timer_set_ticks(iTicks & 0xFF);
	sei();
}