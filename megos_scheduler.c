/*
* megos_scheduler.c
*
* Created: 1/11/2018 9:49:39 PM
*  Author: Matthew
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "megos_system.h"
#include "megos_scheduler.h"
#include "atmel328P.h"

// Start at two because the default task is 1.
static unsigned int task_id_pool = 2;

static struct mos_tcb tcb_main
=  {
   .stack_pointer = ATMEL328P_STACK_START,
   .status = TASK_READY,
   .next_task = 0,
   .task_id = 1,
   .memory_end = 0
};
static struct mos_tcb* current_task = &tcb_main;

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
   scheduler_task_stack_push(aptTask, 1 << 8);

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
   tcb->status = TASK_READY; // TODO: Set to init.
   tcb->next_task = 0;
   tcb->task_id = task_id_pool++;
   tcb->memory_end = tcb->stack_pointer;

   // Queue up.
   scheduler_get_head_task()->next_task = tcb;

   scheduler_initialize_stack(tcb, aptTask);
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
   if(next)
   {
      if(next->status == TASK_READY)
      {
         return next;
      }
      else
      {
         return scheduler_next_ready_task(next);
      }
   }
   else
   {
      return &tcb_main;
   }
}

static void scheduler_task_exit(void)
{
   // Change task to done. Delete it.
   current_task->status = TASK_DONE;

   // This task should not be re-scheduled.
   // but in case it does.
   // TODO: Maybe force call the scheduler?
   while(1);
}

static void scheduler_cleanup_tasks(void)
{
   // Interrupts cannot occur here because the task states are undetermined
   // while this is running..
   // TODO: Perhaps only loop a fixed number of times to minimize blocking time.
   while(1)
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
         megos_task_sleep(0);
      }
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
      scheduler_initialize_tcb(memory_block, aptTask, aiSize);
   }
   // TODO: Assign IDs.
   return 1;
}

void megos_task_sleep(unsigned int aiMilliseconds)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      current_task->sleep_tick_count = megos_millis_get_ticks(aiMilliseconds);
      current_task->status = TASK_SLEEP;
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
   megos_new_task(&scheduler_cleanup_tasks, 196);
}