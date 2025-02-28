#include "../include/keypad.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include "hardware/gpio.h"
#include <stdint.h>
#include <string.h>
#include "pico/time.h"

int button_pins[] = {11, 10, 12, 13, 15, 14};
int common_pins[] = {2, 3};

struct Key keypad[12];
struct Key keypad_last_frame[12];
uint32_t times[12];

void keypad_init() {
    for (int i = 0; i < 6; i++) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);
    }
    for (int i = 0; i < 2; i++) {
        gpio_init(common_pins[i]);
        gpio_set_dir(common_pins[i], GPIO_OUT);
        gpio_put(common_pins[i], true);
    }
    memset(times, 0, 12 * sizeof(uint32_t));
}

bool irq_update_keypad(repeating_timer_t *_) {
    keypad_read(keypad, keypad_last_frame);
    return true;
}

void keypad_next_frame() {
    memcpy(keypad_last_frame, keypad, sizeof(keypad));
    // memset(keypad, 0, sizeof(keypad));
    for (int i = 0; i < 12; i++) {
        keypad[i].pressed = false;
        keypad[i].released = false;
        if (keypad[i].held) {
            keypad[i].f_pressed += 1;
        } else {
            keypad[i].f_pressed = 0;
        }
    }
}

inline void set_key(int half_x, int half_y, int i, uint32_t now, struct Key *pad, struct Key *last_pad) {
    int idx = half_y * 4 + half_x;
    bool state = !gpio_get(button_pins[i]);
    if (!state && now - times[idx] < 10) {
        return;
    }
    if (state) {
        times[idx] = now;
    }
    pad[idx].held = state;
    if (!state) {
        pad[idx].f_pressed = 0;
    }
    if (state && !last_pad[idx].held) {
        pad[idx].pressed = true;
    }
    if (!state && last_pad[idx].held) {
        pad[idx].released = true;
    }
}

void keypad_read(struct Key *pad, struct Key *last_pad) {
    gpio_put(common_pins[0], true);
    gpio_put(common_pins[1], false);
    sleep_us(10);
    uint32_t now = to_ms_since_boot(get_absolute_time());
    for (int i = 0; i < 6; i++) {
        int half_x = i % 2;
        int half_y = i / 2;
        set_key(half_x, half_y, i, now, pad, last_pad);
    }

    gpio_put(common_pins[0], false);
    gpio_put(common_pins[1], true);
    sleep_us(10);
    now = to_ms_since_boot(get_absolute_time());
    for (int i = 0; i < 6; i++) {
        int half_x = i % 2 + 2;
        int half_y = i / 2;
        set_key(half_x, half_y, i, now, pad, last_pad);
    }
}

struct Key keypad_get(int x, int y) {
    return keypad[y * 4 + x];
}
