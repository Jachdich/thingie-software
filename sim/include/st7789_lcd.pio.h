#ifndef _INCLUDE_ST7789_LCD_PIO_H_
#define _INCLUDE_ST7789_LCD_PIO_H_
#include "hardware/pio.h"
extern int st7789_lcd_program;
void st7789_start_pixels(PIO pio, unsigned int sm);
#endif
