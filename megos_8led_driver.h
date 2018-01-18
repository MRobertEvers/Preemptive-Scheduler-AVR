/*
 * megos_8led_driver.h
 *
 * Created: 1/18/2018 10:56:10 AM
 *  Author: Matthew
 */ 


#ifndef MEGOS_8LED_DRIVER_H_
#define MEGOS_8LED_DRIVER_H_

void megos_8led_init(void);
void megos_8led_display_set_row(unsigned char aiRow, unsigned char aidata);
void megos_8led_draw_refresh(void);


#endif /* MEGOS_8LED_DRIVER_H_ */