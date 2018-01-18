# Preemptive-Scheduler-AVR
Preemptive scheduler written specifically for Atmega328P microcontroller (Arduino UNO). 

Specifications
==============
1. Utilizes round-robin scheduling with fixed time slices.  
2. Task control blocks(TCBs) maintained in linked list.  
3. TCBs used indicate task state and task stack location.   
4. Provides Semaphore implementation with special no-starve functions.
5. Provides task sleep and task delay functions.

In-Work
=======
Operating system features in work.  
Better memory handling.