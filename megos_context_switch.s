
/*
 * megos_context_switch.s
 *
 * Created: 1/12/2018 4:41:11 PM
 *  Author: Matthew
 */ 

 ; IO mapped addresses of these registers. See atmega328P manual.
 #define _SREG 0x3F
 #define _SPH 0x3E
 #define _SPL 0x3D

 ; Declare this function visible for the linker.
 .global megos_context_switch

 ; megos_context_switch( void** save, void** switch )
 ;
 ; @param save: Pointer to the memory location that the stack pointer of the
 ;              context that enters this function.
 ;    		  : void** two bytes -> low byte r24; high byte r25
 ;
 ; @param switch: See save. Context that leaves this function.
 ;              : void** two bytes -> low byte r22; high byte r23
 ;
 ; Saves the state of the registers (for the context switch function) 
 ; on the stack and then saves the stack
 ; pointer of the current context in address pointed to by argument one,
 ; then loads the stack context starting with the stack pointer stored at
 ; the location pointed to by argument 2.
 ; Also, note that the interrupt must save the registers. This is a different
 ; routine
 ; Megos_contexts.h contains c declaration of this functions
 megos_context_switch:
    ; Save the current context.
	; The return address is pushed onto the stack just before this function
	; as part of the function call. (See avr-gcc ABI documentation)
	; Thus, it is expected that this function be tail called/ close to
	; tail called (although this is not required). This is expected so that
	; the task will return to its main task as soon as the context switch occurs.
	in r0, _SREG ; Save the Status Register near the 'bottom' of the stack.
				 ; We want it to be one of the last things restored.
				 ; Access the Status Register via its IO mapped address.
				 ; This is 0x3F. See atmel manual 11.3.1
	push r0      ; Put the SREG above the GPRs on the stack.
				 ; Now we want the stack pointer to be in the location provided as
				 ; an argument.
	push r31
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
	mov r26, r24 ; See atmel manual 11.4. Load X (low byte) with SP Destination low byte
	mov r27, r25
	in r0, _SPL
	st X+, r0    ; X+ is post-increment. This stores the value of _SPH at the address X and
				 ; then increments X
	in r0, _SPH  
	st X, r0
	; Load the new context.
	; See "Frame Layout" of the ABI in the avr-gcc documentation.
	mov r26, r22    ; Get the address that the second  argument pointer is pointing to.
	mov r27, r23
	ld r22, X+
	ld r23, X
	out _SPL, r22	; Now our stack pointer is pointing at the location that we previously
	out _SPH, r23   ; stored the context for this task, so we can start popping.
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
	pop r0
	out _SREG, r0
	ret ; Re-enable interupts (part of reti)