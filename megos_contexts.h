/*
 * megos_contexts.h
 *
 * Created: 1/18/2018 10:05:57 PM
 *  Author: Matthew
 */ 


#ifndef MEGOS_CONTEXTS_H_
#define MEGOS_CONTEXTS_H_

/*
* megos_interrupt_schedule(void)
*
* See megos_interrupt_handle.s for details.
*/
void megos_interrupt_schedule(void);

/*
* megos_context_switch(void**,void** )
*
* See megos_context_switch.s for details.
*/
void megos_context_switch(void** aspsave,void** aspswitch );



#endif /* MEGOS_CONTEXTS_H_ */