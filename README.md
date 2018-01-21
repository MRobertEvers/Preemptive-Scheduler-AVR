# Preemptive-Scheduler-AVR
Preemptive scheduler written specifically for Atmega328P microcontroller (Arduino UNO). 

Specifications
==============
1. Utilizes round-robin scheduling with fixed time slices.  
2. Task control blocks(TCBs) maintained in linked list.  
3. TCBs used indicate task state and task stack location.   
4. Provides Semaphore implementation with special no-starve functions.
5. Provides task sleep and task delay functions.

8x8 LED Driver
==============
The driver supports a two-shift register setup. The shift registers must have at least 8 parallel outputs each, and at least one must have be latched. (i.e. the ability to hold output until the latch is triggered).  
Additionally, it helps to have some transistor arrays, as one of the shift registers' output must ground the input to the LED matrix.

On Arduino UNO Board, PORTB and PORTD are used.
On PORTD  
1. Pin 2 (PORTD2) is the Latch of hi source shift register. (The register that provides the voltage to the LED matrix)  
2. Pin 3 (PORTD3) is the serial input of the hi source shift register.  
3. Pin 4 (PORTD4) is the clock source for the hi source shift register.  
On PORTB
1. Pin 11 (PORTB3) is the serial input of the gnd source shift register. (The register that provides the ground to the LED matrix rows)
2. Pin 12 (PORTB4) is the clock source of the gnd source shift register.  
  
Of course, the GND and 5V pins are used to provide the circuit.  
One possible setup includes the use of transistor arrays (or just 8 transistors) to turn the output from the ground source shift register into a ground source for the LED matrix.
(i.e. The base of the transistor is the output of the shift register, the emitter is the input to the LED matrix expecting ground, and the collector is ground)  

For documentation of my own setup, I am using the KWM-30881CWB 8x8 LED matrix from Adafruit. Its pinouts are  
(From the top of the matrix with the 'nub' and serial number facing down)  
0  1  2  3  4  5  6 7  
15 14 13 12 11 10 9 8  
0: C7  
1: C6  
2: R1  
3: C0  
4: R3  
5: C5  
6: C3  
7: R0  
8: R2  
9: R5  
10: C4  
11: R7  
12: C2  
13: C1  
14: R6  
15: R4  
All 'R'ows connect to ground.  
All 'C'olumns connect to V+.


In-Work
=======
Operating system features in work.  
Better memory handling.