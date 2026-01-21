#ifndef _KEYPAD_H
#define _KEYPAD_H
#include <stdbool.h>
#include "pico/time.h"
struct Key {
    bool held;
    bool pressed;
    bool released;
    int f_pressed;
    long us_pressed_at;
};

// extern struct Key keypad[12];
void keypad_init();
void keypad_read();
struct Key keypad_get(int x, int y);
bool irq_update_keypad(repeating_timer_t *);
void keypad_next_frame();

#endif
