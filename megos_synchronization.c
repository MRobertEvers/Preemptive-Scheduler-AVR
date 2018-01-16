/*
 * megos_synchronization.c
 *
 * Created: 1/13/2018 9:05:35 AM
 *  Author: Matthew
 */ 

 /*
 * Notes on atomic.h
 * This header uses some tricky stuff to allow for nice
 * atomic syntax. Basically, it utilizes a 'for' loop
 * scope that iterates once. It initializes a variable, sreg_save, in the
 * loop declaration that stores the SREG state and utilizes
 * a special compiler variable attribute "__cleanup" that calls
 * a special function when sreg_save goes out of scope. 
 * See documentation of __cleanup attribute in the gcc docs.
 * The special function indicated by the macro is included in this 
 * header, this special function restores the sreg state.
 */
#include <util/atomic.h>
#include "megos_synchronization.h"
#define MAX_SEMAPHORES 10

static inline void sem_block_if_neg(semaphore sem)
{
   // Interrupts must have already been disabled,
   // otherwise 2 sems could decrement, then compare and both fail.
   while(1)
   {
      cli();
      if(*sem >= 0)
      {
         // We don't need to enabled interrupts here.
         // The outer function MUST enable interrupts if
         // desired.
         break;
      }
      // Interrupts MUST be enabled here.
      // If we are blocking, then interrupts are the
      // only way out of this loop.
      sei();
   }
}

// TODO: Avoid static allocation
static int semaphores[MAX_SEMAPHORES];
static unsigned int current_sem = 0;
int* megos_new_sem(unsigned int sem_val)
{
	semaphores[current_sem] = sem_val;
	return &semaphores[current_sem++];
}

void megos_del_sem(semaphore sem)
{
	if(sem >= semaphores && sem < semaphores+current_sem)
	{
		int* ptr = sem;
		while(ptr < semaphores+current_sem)
		{
			*ptr = *(ptr+1);
         ptr++;
		}
		current_sem--;
	}
}

void megos_sem_V(semaphore sem)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		(*sem)++;
	}
}

void megos_sem_P(semaphore sem)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      (*sem)--;
      sem_block_if_neg(sem);
   }
}