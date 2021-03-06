
/*
 * megos_interrupt_handle.s
 *
 * Created: 1/18/2018 2:31:52 PM
 *  Author: Matthew
 */ 

  .global megos_interrupt_schedule

 ; void megos_interrupt_schedule(void)
 ;
 ; Pushes each of the registers onto the stack before calling
 ; the C interrupt handler. This is because the interrupt does
 ; not automatically do so for all registers.
 ; Megos_contexts.h contains c declaration of this functions
 megos_interrupt_schedule:
   push r31 ;Save all the registers from before the interrupt
	push r30
	push r29
	push r28
	push r27
	push r26
	push r25
	push r24
	push r23
	push r22
	push r21
	push r20
	push r19
	push r18
	push r17
	push r16
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push r7
	push r6
	push r5
	push r4
	push r3
	push r2
	push r1
	push r0
   ; Now call the c megos_schedule function with its argument of 1.
   ldi r25, 0
   ldi r24, 1
   call megos_schedule
   ; Restore the registers that may have been clobbered in the interrupt.
   pop r0
	pop r1
	pop r2
	pop r3
	pop r4
	pop r5
	pop r6
	pop r7
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	pop r16
	pop r17
	pop r18
	pop r19
	pop r20
	pop r21
	pop r22
	pop r23
	pop r24
	pop r25
	pop r26
	pop r27
	pop r28
	pop r29
	pop r30
	pop r31
   ret
