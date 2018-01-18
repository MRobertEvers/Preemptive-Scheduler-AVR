/*
 * megos_scheduler.h
 *
 * Created: 1/11/2018 9:50:07 PM
 *  Author: Matthew
 */ 


#ifndef MEGOS_SCHEDULER_H_
#define MEGOS_SCHEDULER_H_

typedef void (*mos_task_fn)(void);

/*
* megos_new_task(mos_task_fn , unsigned int)
* 
* Creates a new task with entry point aptTask and stack size of aiSize.
* Creates the task in the init state. The task must be started before
* it will be scheduled to run.
*
* @Param aptTask: Entry point for the task.
* @Param aiSize: Stack size.
* @Return: Unique identifier for the new task.
*/
unsigned int megos_new_task(mos_task_fn aptTask, unsigned int aiSize);

/*
* megos_new_task_at(mos_task_fn, void*, unsigned int)
*
* Creates a new task with entry point aptTask and stack size of aiSize and
* at location aptMem. This allows for modules to allocate their own memory
* for a task rather than rely on this tasks' malloc.
* Creates the task in the init state. The task must be started before
* it will be scheduled to run.
*
* @Param aptTask: Entry point for the task.
* @Param aptMem: Location of memory to put task.
* @Param aiSize: Stack size.
* @Return: Unique identifier for the new task.
*/
unsigned int megos_new_task_at(mos_task_fn aptTask, void* aptMem, unsigned int aiSize);

/*
* megos_task_start(unsigned int)
*
* Changes the state of the input task from INIT to READY.
*
* @Param aiTask: Task ID of task to start. Must be in INIT or this does nothing.
*/
void megos_task_start(unsigned int aiTask);

/*
* megos_task_sleep(unsigned int)
*
*/
void megos_task_sleep(unsigned int aiMilliseconds);

/*
* megos_schedule(void)
*
* Determines the next task to be run and performs the context switch.
*/
void megos_schedule(unsigned char abIsInterrupt);

/*
* megos_schedule_control_init(void)
*
* Turns on the process that cleans up other processes. 
* This process should only be called when another process reaches complete.
*/
void megos_schedule_control_init(void);

#endif /* MEGOS_SCHEDULER_H_ */