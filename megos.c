/*
 * megos.c
 *
 * Created: 1/11/2018 9:49:13 PM
 *  Author: Matthew
 */ 

 #include "megos.h"
 #include "megos_system.h"
 #include "megos_scheduler.h"

 void megos_init()
 {
	megos_sched_timer_set(1);
   megos_schedule_control_init();
 }