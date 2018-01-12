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
	TASK_INIT = 0x0
};

static struct mos_tcb
{
	void* stack_pointer;
	struct mos_tcb* next_task;
	void* memory_end;
	unsigned int task_id;
	enum mos_task_status status;
};

static void scheduler_task_free(struct mos_tcb* aptTCB);
static void scheduler_task_stack_push(struct mos_tcb* aptTask, unsigned char acValue);
static struct mos_tcb* scheduler_task_clean_child(struct mos_tcb* aptTCB);
static struct mos_tcb* scheduler_get_head_task(void);
static unsigned int scheduler_calc_task_mem_size(unsigned int aiSize);
static void scheduler_initialize_stack(struct mos_tcb* aptTask, mos_task_fn aptFN);
static struct mos_tcb* scheduler_initialize_tcb(void* aptMemoryStart, mos_task_fn aptTask, unsigned int aiSize);
static void* scheduler_allocate_task_memory(unsigned int aiSize);
static struct mos_tcb* scheduler_next_ready_task(struct mos_tcb* apStartTask);
static void scheduler_task_exit(void);
static void scheduler_cleanup_tasks(void);
static void scheduler_context_switch(void** current_context, void** next_context);

unsigned int megos_new_task(mos_task_fn aptTask, unsigned int aiSize);
void megos_schedule(void);

#endif /* MEGOS_SCHEDULER_H_ */