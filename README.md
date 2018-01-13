# Preemptive-Scheduler-AVR
Preemptive Scheduler for Atmega328P.

Preemptive scheduler written specifically for Atmega328P microcontroller (Arduino UNO). 

Specifications
==============
Utilizes round-robin scheduling with fixed time slices.
Task control blocks(TCBs) maintained in linked list.
TCBs used indicate task state and task stack location. 
