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

static void sched_timer_enable_CTC(void);
static void sched_timer_set_ticks(unsigned char aiTicks);
static void sched_timer_enable(void);

void megos_sched_timer_set(int aiMilliseconds);



#endif /* MEGOS_SYSTEM_H_ */