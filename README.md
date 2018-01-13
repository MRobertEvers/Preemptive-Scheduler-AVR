# Preemptive-Scheduler-AVR
Preemptive scheduler written specifically for Atmega328P microcontroller (Arduino UNO). 

Specifications
==============
1. Utilizes round-robin scheduling with fixed time slices.  
2. Task control blocks(TCBs) maintained in linked list.  
3. TCBs used indicate task state and task stack location.   

In-Work
=======
Operating system features in work.  
Future additions will include semaphores, task sleep capabilities, and a malloc/free specialized to allocate within task memory blocks.