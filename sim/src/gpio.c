#include <stdio.h>
void stdio_init_all() {}
void gpio_set_dir() {}
void gpio_set_function() {}
int pwm_gpio_to_slice_num() {return 0;}
void pwm_set_wrap() {}
void pwm_set_enabled() {}
void gpio_init() {}
void gpio_pull_up() {}

int half = 0;
void gpio_put(int pin, int state) {
    if (pin == 2 && state) {
        half = 1;
    }
    if (pin == 3 && state) {
        half = 0;
    }
}

int __test_keypad[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

int pin_buttons[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 5, 4
};

int gpio_get(int pin) {
    if (pin > 15 || pin < 10) return 0;
    int idx = pin_buttons[pin] + half * 6;
    return __test_keypad[idx];
}

void gpio_put_masked() {}
void st7789_lcd_wait_idle() {}
void st7789_lcd_program() {}
void pio_add_program() {}
void st7789_lcd_program_init() {}

