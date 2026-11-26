#ifndef _ST7789_PIO_H
#define _ST7789_PIO_H
#include "hardware/pio.h"
#include <stdbool.h>
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
typedef struct {
    PIO pio;
    int sm;
} ST7789;
void st7789_init();
void lcd_set_dc_cs(bool dc, bool cs);
#endif
