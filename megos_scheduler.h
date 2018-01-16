/*
 * megos_scheduler.h
 *
 * Created: 1/11/2018 9:50:07 PM
 *  Author: Matthew
 */ 


#ifndef MEGOS_SCHEDULER_H_
#define MEGOS_SCHEDULER_H_

typedef void (*mos_task_fn)(void);

static enum mos_task_status
{
	TASK_READY = 0x1,
	TASK_WAIT = 0x2,
	TASK_DONE = 0x4,
   TASK_SLEEP = 0x8,
	TASK_INIT = 0x0
};

static struct mos_tcb
{
	void* stack_pointer;
	struct mos_tcb* next_task;
	void* memory_end;
	unsigned int task_id;
   unsigned long sleep_tick_count;
	enum mos_task_status status;
};

/* scheduler_task_free(struct mos_tcb*)
*  
* Frees the memory allocated for task controlled by the input task control block.
*
* @Param aptTCB: TCB for the task to be freed.
*/
static void scheduler_task_free(struct mos_tcb* aptTCB);

/* scheduler_task_stack_push(struct mos_tcb* aptTask, unsigned char acValue)
* 
* Pushes value 'acValue' onto the stack pointed to by 'aptTask'. Decrements the stack
* pointer of the TCB in concordance with the behavior of actually pushing values to the
* stack.
*
* @Param aptTask: Task whose stack has 'acValue' pushed to it.
* @Param acValue: Value to push to aptTask's stack.
*/
static void scheduler_task_stack_push(struct mos_tcb* aptTask, unsigned char acValue);

/*
* scheduler_task_clean_child(struct mos_tcb*)
*
* Returns the next task in the task queue (starting with aptTCB) that is ready 
* to be run, and frees all TCBs between 'aptTCB' and the next non-complete TCB.
* Adjusts the linked list accordingly.
*
* @Param aptTCB: Starting point in the queue to start looking for the next task.
* @return: The next struct that is not complete after and including aptTCB in
*          the queue.
*/
static struct mos_tcb* scheduler_task_clean_child(struct mos_tcb* aptTCB);

/* 
* scheduler_get_head_task(void)
*
* Gets the last element in the task list.
*
* @Return: The last TCB in the task list.
*/
static struct mos_tcb* scheduler_get_head_task(void);

/* 
* scheduler_calc_task_mem_size(unsigned int)
* 
* Calculates the desired memory size by including the memory required
* to also store a TCB.
*
* @Return: The adjusted size that include room for a TCB.
*/
static unsigned int scheduler_calc_task_mem_size(unsigned int aiSize);

/*
* scheduler_initialize_stack(struct mos_tcb* aptTask, mos_task_fn aptFN)
* 
* Populates the (Newly-created) stack of the input TCB with the required info 
* to be switched into. 
*
* @Param aptTask: TCB whose stack is to be populated.
* @Param aptFN: Entry point function of this task.
*
* Notes: See avr-gcc ABI and ATmega328P manual 11.7.1 for stack behavior.
* Puts the return address of the function on the stack so that when 
* the context switch function returns, it will return to the target
* function.
*/
static void scheduler_initialize_stack(struct mos_tcb* aptTask, mos_task_fn aptFN);

/*
* scheduler_initialize_tcb(void* , mos_task_fn , unsigned int );
*
* Creates a TCB struct at the input pointer. Populates the fields of the TCB
* and sets the task to the INIT state. Initializes the stack at the end of the
* memory region provided (see scheduler_initialize_stack).
*
* @Param aptMemoryStart: Pointer to the start location of the task's memory block.
* @Param aptTask: Entry point of the new task.
* @Param aiSize: Size of the memory allocated for the stack of the task. (does not
*                include the size of the tcb struct.
* @Return: Pointer to the tcb created. (Really just a casted pointer)
*/
static struct mos_tcb* scheduler_initialize_tcb(void* aptMemoryStart, mos_task_fn aptTask, unsigned int aiSize);

/*
* scheduler_allocate_task_memory(unsigned int)
*
* Allocates the memory required for a task to have a stack size of aiSize.
* This also allocates space of size of TCB.
* @Param aiSize: Desired size of stack.
* @Retrun: Pointer to the beginning of allocated memory. 
*/
static void* scheduler_allocate_task_memory(unsigned int aiSize);

/*
* scheduler_next_ready_task(struct mos_tcb*)
* 
* Start with the input TCB, it searches through the linked list for the next
* Ready TCB. Loops to the beginning of the list.
*
* @Param aptStartTask: Task in the linked list to start search after.
* @Return: Next task in the linked list marked as ready.
*/
static struct mos_tcb* scheduler_next_ready_task(struct mos_tcb* apStartTask);

/*
* scheduler_find_task(unsigned int, struct mos_tcb**)
*
* Tries to find the task with the input ID. If successful, the task is returned
* in the rStruct.
*
* @Param aiTask: Task to find.
* @Param rStruct: Return value if successful.
* @Return: 1 if successful, 0 otherwise.
*/
static unsigned char scheduler_find_task(unsigned int aiTask, struct mos_tcb** rStruct);

/*
* scheduler_task_wakeup(unsigned int)
*
* Changes the state of the input task to READY and removes any
* sleep time. Only does this if the task is either WAITING or SLEEPING
*
* @Param aiTask: Task to wake.
*/
static void scheduler_task_wakeup(unsigned int aiTask);

/*
* scheduler_task_exit(void)
*
* Wrapper function for all created tasks. When the entry point function
* for a task exits, it will return to this function. This function
* sets the running task to done so it will not be scheduled again.
*/
static void scheduler_task_exit(void);

/*
* scheduler_cleanup_tasks(void)
*
* Iterates through each TCB in the linked list and removes finished
* tasks.
*/
static void scheduler_cleanup_tasks(void);

/*
* scheduler_context_switch(void**, void**)
*
* Performs the context switch between the stack pointers pointed to by
* the first and second arguments.
*
* @Param current_context: Pointer to the address to store the current tasks
*                         stack pointer.
* @Param next_context: Pointer to the address where the next task's stack
*                      pointer is located.
*
* Notes: This calls an assembly function. The context will be different upon
* exiting this function.
*/
static void scheduler_context_switch(void** current_context, void** next_context);

/*
* scheduler_wake_sleeping_tasks_if_time(void)
*
* Checks all the tasks and decrements their sleep time.
* If a sleep time is 0, then it wakes the task.
*/
static void scheduler_wake_sleeping_tasks_if_time(void);

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

#endif /* MEGOS_SCHEDULER_H_ */