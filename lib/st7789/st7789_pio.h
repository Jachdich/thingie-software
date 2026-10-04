#ifndef _ST7789_PIO_H
#define _ST7789_PIO_H
#include "hardware/pio.h"
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
typedef struct {
    PIO pio;
    int sm;
} ST7789;
void st7789_init();
#endif
