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
 
 /*
 * driver_8led_flip(void)
 * 
 * Flips the double buffer to update the display.
 */
 static void driver_8led_flip(void);

  /*
 * driver_8led_draw(void)
 *
 * Requires output to be on Port D; pins 0-7.
 * Requires serial output on Port B; PORTB3.
 * Requires RCLK and SRCLK to be jumpered and on Port B; PORTB4
 */
 static void driver_8led_draw(void);

 /*
 * driver_8led_start_sweep(void)
 * 
 * Starts the shift register. Each shift after this will
 * shift the register one notch.
 *
 * Note: This register exists in the configuration required to use this
 * driver. It must be present for this driver to work.
 */
 static inline void driver_8led_start_row_sweep(void);

 /*
 * driver_8led_shift(void)
 *
 * Cycles the shift register clock. Shifts each bit in the register.
 */
 static inline void driver_8led_shift_row(void);

 /*
 * driver_8led_set_row(unsigned char aiRow)
 *
 * Sets each of the columns in the current row equal
 * to the bits in aiRow.
 */
 static inline void driver_8led_set_row(unsigned char aiRow);

 /*
 *
 */
 static inline void driver_8led_shift_column(void);

 static inline void driver_8led_show_row(void);

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
         driver_8led_start_row_sweep();
         driver_8led_set_row(display_buffer[0]);
         driver_8led_set_row(0);
         for(int i = 1; i < 8; i++)
         {
            driver_8led_shift_row();
            driver_8led_set_row(display_buffer[i]);
            driver_8led_set_row(0);
         }
         megos_sem_V(buffer_sem);
      }
    }
 }

 static void driver_8led_start_row_sweep(void)
 {
   PORTB = (1 << PORTB3);
   driver_8led_shift_row();
   PORTB &= !(1 << PORTB3);
 }

 static void driver_8led_shift_row(void)
 {
    PORTB |= (1 << PORTB4);
    megos_delay_mus(10);
    PORTB &= !(1 << PORTB4);
    megos_delay_mus(10);
 }

 static void driver_8led_set_row(unsigned char aiRow)
 {
   
   for(int i = 0; i < 8; i++)
   { 
      PORTD = ((0x1 & (aiRow>>(7-i))) << PORTD3);
      driver_8led_shift_column();
   }
   driver_8led_show_row();
 }

static void driver_8led_shift_column(void)
{
   PORTD |= (1 << PORTD4);
   megos_delay_mus(10);
   PORTD &= !(1 << PORTD4);
   megos_delay_mus(10);
}

 static void driver_8led_show_row(void)
 {
    PORTD = (1 << PORTD2);
    megos_delay_mus(10);
    PORTD &= !(1 << PORTD2);
    megos_delay_mus(10);
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