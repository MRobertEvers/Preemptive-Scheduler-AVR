/*
 * megos_8led_driver.c
 *
 * Created: 1/18/2018 10:55:54 AM
 *  Author: Matthew
 */ 

 #include <util/atomic.h>
 #include "megos_scheduler.h"
 #include "megos_synchronization.h"
 #include "megos_settings.h"
 #include "megos_system.h"
 #include "megos_8led_driver.h"

 // Since a char is 8 bits long, we can do an array of 8 chars.
 static unsigned char draw_task_memory[SCHEDULER_DEFAULT_TASK_SIZE];
 static unsigned char display_buffer_front[8];
 static unsigned char display_buffer_back[8];
 static unsigned char* display_buffer = display_buffer_front;
 static unsigned char* write_buffer = display_buffer_back;
 static int* buffer_sem = 0;
 static int* write_buffer_sem = 0;
 static int* stop_starve_sem = 0;

 static void driver_8led_flip(void);

  /*
 * driver_8led_draw(void)
 *
 * Requires output to be on Port D; pins 0-7.
 * Requires serial output on Port B; PORTB3.
 * Requires RCLK and SRCLK to be jumpered and on Port B; PORTB4
 */
 static void driver_8led_draw(void);
 static void driver_8led_start_sweep(void);
 static void driver_8led_shift(void);

 static void driver_8led_flip(void)
 {
   megos_sem_P_stop_starve(buffer_sem, stop_starve_sem);
   megos_sem_P(write_buffer_sem);
   if( display_buffer == display_buffer_front )
   {
      display_buffer = display_buffer_back;
      write_buffer = display_buffer_front;
   }
   else
   {
      write_buffer = display_buffer_back;
      display_buffer = display_buffer_front;
   }
   megos_sem_V(write_buffer_sem);
   megos_sem_V(buffer_sem);
 }

 static void driver_8led_draw(void)
 {
    while(1)
    {
      megos_sem_P_stop_starve(buffer_sem, stop_starve_sem);
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
         driver_8led_start_sweep();
         for(int i = 0; i < 8; i++)
         {
            PORTD = (display_buffer[i] << PORTD2);
            driver_8led_shift();
         }
         megos_sem_V(buffer_sem);
      }
    }
 }

 static void driver_8led_start_sweep(void)
 {
   PORTB = (1 << PORTB3);
   driver_8led_shift();
   PORTB &= !(1 << PORTB3);
 }

 static void driver_8led_shift(void)
 {
    PORTB |= (1 << PORTB4);
    megos_delay_ns(10);
    PORTB &= !(1 << PORTB4);
    megos_delay_ns(10);
 }

 void megos_8led_init(void)
 {
   DDRB |= 0x18;
   DDRD |= 0xFF;
   buffer_sem = megos_new_sem(1);
   write_buffer_sem = megos_new_sem(1);
   stop_starve_sem = megos_new_sem(1);
   int iTask = megos_new_task_at(&driver_8led_draw, draw_task_memory, SCHEDULER_DEFAULT_TASK_SIZE);
   megos_task_start(iTask);
 }

 void megos_8led_display_set_row(unsigned char aiRow, unsigned char aidata)
 {
   if( aiRow < 8 )
   {
      megos_sem_P(write_buffer_sem);
      write_buffer[aiRow] = aidata;
      megos_sem_V(write_buffer_sem);
   }
 }

 void megos_8led_draw_refresh(void)
 {
   driver_8led_flip();
 }