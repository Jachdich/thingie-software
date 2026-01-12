#ifndef _ST7789_PIO_H
#define _ST7789_PIO_H
#include "st7789_lcd.pio.h"
#include "hardware/pio.h"
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
typedef struct {
    PIO pio;
    int sm;
} ST7789;
ST7789 st7789_init();
#endif