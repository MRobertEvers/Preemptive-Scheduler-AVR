/*
* megos_scheduler.c
*
* Created: 1/11/2018 9:49:39 PM
*  Author: Matthew
*/

#include <avr/io.h>
#include <limits.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "megos_system.h"
#include "megos_scheduler.h"
#include "atmel328P.h"
#include <stdlib.h>


enum mos_task_status
{
   TASK_READY = 0x1,
   TASK_WAIT = 0x2,
   TASK_DONE = 0x4,
   TASK_SLEEP = 0x8,
   TASK_INIT = 0x0
};

struct mos_tcb
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


//////////////////////////////////////////////////////////////////////////
///////////////////////////////Implementation/////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Start at two because the default task is 1.
static unsigned int task_id_pool = 1;
static struct mos_tcb tcb_main
=  {
   .stack_pointer = ATMEL328P_STACK_START,
   .status = TASK_READY,
   .next_task = 0,
   .task_id = 0,
   .memory_end = 0
};
static struct mos_tcb* current_task = &tcb_main;
static unsigned int cleanup_task = 0;

static void scheduler_task_free(struct mos_tcb* aptTCB)
{
   free((void*)aptTCB);
}

static void scheduler_task_stack_push(struct mos_tcb* aptTask, unsigned char acValue)
{
   *(char*)aptTask->stack_pointer = acValue;
   aptTask->stack_pointer--;
}

static struct mos_tcb* scheduler_task_clean_child(struct mos_tcb* aptTCB)
{
   if(!aptTCB)
   {
      return 0;
   }
   else if(aptTCB->status == TASK_DONE)
   {
      struct mos_tcb* next = aptTCB->next_task;
      scheduler_task_free(aptTCB);
      return scheduler_task_clean_child(next);
   }
   else
   {
      return aptTCB;
   }
}

static struct mos_tcb* scheduler_get_head_task(void)
{
   struct mos_tcb* task = &tcb_main;
   while(task->next_task)
   {
      task = task->next_task;
   }
   return task;
}

static unsigned int scheduler_calc_task_mem_size(unsigned int aiSize)
{
   return aiSize+sizeof(struct mos_tcb);
}

static void scheduler_initialize_stack(struct mos_tcb* aptTask, mos_task_fn aptFN)
{
   // Put the cleanup function at the bottom of the stack. When the aptTask returns,
   // it will return to the scheduler_cleanup. Low bits go towards the bottom of the
   // stack. (high in memory)
   scheduler_task_stack_push(aptTask, (int)&scheduler_task_exit & 0xFF);
   scheduler_task_stack_push(aptTask, ((int)&scheduler_task_exit & 0xFF00) >> 8);

   // Put the entry point on the stack.
   scheduler_task_stack_push(aptTask, (int)aptFN & 0xFF);
   scheduler_task_stack_push(aptTask, ((int)aptFN & 0xFF00) >> 8);
   
   // Put the status register on the stack.
   scheduler_task_stack_push(aptTask, 1 << 7);

   // Put the 32 general purpose registers on the stack.
   for(uint8_t i = 0; i < 32; i++)
   {
      scheduler_task_stack_push(aptTask, 0);
   }
}

static struct mos_tcb* scheduler_initialize_tcb(void* aptMemoryStart, mos_task_fn aptTask, unsigned int aiSize)
{
   struct mos_tcb* tcb = aptMemoryStart;

   // Point the stack pointer to the highest memory value in the block. The stack grows down.
   tcb->stack_pointer = aptMemoryStart + scheduler_calc_task_mem_size(aiSize);
   tcb->status = TASK_INIT;
   tcb->next_task = 0;
   tcb->task_id = task_id_pool++;
   tcb->memory_end = tcb->stack_pointer;

   // Get the last task in the queue. This comes after that.
   scheduler_get_head_task()->next_task = tcb;

   scheduler_initialize_stack(tcb, aptTask);

   return tcb;
}

static void* scheduler_allocate_task_memory(unsigned int aiSize)
{
   // TODO: Implement my own malloc.
   return malloc(scheduler_calc_task_mem_size(aiSize));
}

static struct mos_tcb* scheduler_next_ready_task(struct mos_tcb* apStartTask)
{
   // This is round robin style selection.
   struct mos_tcb* next = apStartTask->next_task;
   while(next && next->status != TASK_READY)
   {
      next = next->next_task;
   }

   if(next)
   {
      return next;
   }
   else
   {
      return &tcb_main;
   }
}

static unsigned char scheduler_find_task(unsigned int aiTask, struct mos_tcb** rStruct)
{
   struct mos_tcb* next = &tcb_main;
   while(next && next->task_id != aiTask)
   {
      next = next->next_task;
   }

   if(next)
   {
      *rStruct = next;
      return 1;
   }
   else
   {
      return 0;
   }
}

static void scheduler_task_wakeup(unsigned int aiTask)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      struct mos_tcb* next;
      if( scheduler_find_task(aiTask, &next) )
      {
         if( next->status == TASK_SLEEP || next->status == TASK_WAIT )
         {
            next->sleep_tick_count = 0;
            next->status = TASK_READY;
         }
      }
   }
}

static void scheduler_task_exit(void)
{
   // Change task to done. Delete it.
   current_task->status = TASK_DONE;
   scheduler_task_wakeup(cleanup_task);
   while(1)
   {
      megos_schedule(0);
   }
}

static void scheduler_cleanup_tasks(void)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      // TCB main should never be done until end of execution
      struct mos_tcb* next = &tcb_main;
      while(next)
      {
         next->next_task = scheduler_task_clean_child(next->next_task);
         next = next->next_task;
      }
      megos_task_sleep(USHRT_MAX);
   }
}

static void scheduler_context_switch(void** current_context_sp, void** next_context_sp)
{
   // Set the current task before calling this function.
   // That should be done atomically with this function.
   megos_context_switch(current_context_sp, next_context_sp);
}

static void scheduler_wake_sleeping_tasks_if_time()
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      struct mos_tcb* next = &tcb_main;
      while(next)
      {
         next->sleep_tick_count -= 1 * (next->sleep_tick_count != 0);
         if(next->status == TASK_SLEEP && next->sleep_tick_count == 0)
         {
            next->status = TASK_READY;
         }
         next = next->next_task;
      }
   }
}

unsigned int megos_new_task(mos_task_fn aptTask, unsigned int aiSize)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      void* memory_block = scheduler_allocate_task_memory(aiSize);
      struct mos_tcb* new_tcb = scheduler_initialize_tcb(memory_block, aptTask, aiSize);
      return new_tcb->task_id;
   }
   return 0;
}

void megos_task_start(unsigned int aiTask)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      struct mos_tcb* next;
      if( scheduler_find_task(aiTask, &next) )
      {
         if(next->status == TASK_INIT)
         {
            next->status = TASK_READY;
         }
      }
   }
}

void megos_task_sleep(unsigned int aiMilliseconds)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      current_task->sleep_tick_count = megos_millis_get_ticks(aiMilliseconds);
      current_task->status = USHRT_MAX == aiMilliseconds ? TASK_WAIT : TASK_SLEEP;
      megos_schedule(0);
   }
}

void megos_schedule(unsigned char abIsInterrupt)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      if(abIsInterrupt)
      {
         scheduler_wake_sleeping_tasks_if_time();
      }

      struct mos_tcb* next_task = scheduler_next_ready_task(current_task);

      if(next_task != current_task)
      {
         void** curr_sp = &current_task->stack_pointer;
         void** next_sp = &next_task->stack_pointer;
         current_task = next_task;
         scheduler_context_switch(curr_sp, next_sp);
      }
   }
}

void megos_schedule_control_init(void)
{
   cleanup_task = megos_new_task(&scheduler_cleanup_tasks, 196);
   megos_task_start(cleanup_task);
}