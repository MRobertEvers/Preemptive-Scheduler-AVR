/*
 * megos_synchronization.h
 *
 * Created: 1/13/2018 9:05:21 AM
 *  Author: Matthew
 */ 


#ifndef MEGOS_SYNCHRONIZATION_H_
#define MEGOS_SYNCHRONIZATION_H_

typedef int* semaphore;

static void sem_block_if_neg(semaphore sem);

/*
* megos_new_sem(usnigned int)
*
* Allocates a new semaphore. Returns a pointer to it.
*
* @Return: Returns pointer to new semaphore.
*/
int* megos_new_sem(unsigned int sem_val);

/*
* megos_del_sem(semaphore)
* 
* Frees the semaphore if it was allocated by megos_new_sem.
*
* @Param sem: Semaphore to free.
*/
void megos_del_sem(semaphore sem);

/*
* megos_sem_V(semaphore)
* 
* Increment and wake a waiting thread on semaphore.
*
* @Param sem: Semaphore to increment.
*/
void megos_sem_V(semaphore sem);

/*
* megos_sem_P(semaphore)
*
* Decrement and block if *sem is negative.
*
* @Param sem: Semaphore to decrement.
*/
void megos_sem_P(semaphore sem);

#endif /* MEGOS_SYNCHRONIZATION_H_ */