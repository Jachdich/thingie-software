// #include "../include/st7789.h"
#include "../include/drawing.h"
#include "../include/st7789_pio.h"
#include "../include/keypad.h"
#include "../include/snake.h"
#include "../include/minesweeper.h"
#include "pico/time.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// const struct st7789_config lcd_config = {
//     .spi      = spi0,
//     .gpio_din = 19,
//     .gpio_clk = 18,
//     .gpio_cs  = 28,
//     .gpio_dc  = 20,
//     .gpio_rst = 21,
//     .gpio_bl  = 26,
// };
// const int framrate = 60;

uint16_t framebuffer0[240*240];
uint16_t framebuffer1[240*240];
uint16_t *front_buffer;
uint16_t *back_buffer;

uint8_t *game_arena[10*1024]; // just allocate 10MiB for now...

void core1_entry() {
    alarm_pool_t *pool = alarm_pool_create_with_unused_hardware_alarm(1);
    repeating_timer_t timer;
    alarm_pool_add_repeating_timer_ms(pool, 2, irq_update_keypad, NULL, &timer);
    
    uint16_t **buffer = &front_buffer;
    ST7789 st = st7789_init();
    while (1) {
        // long time = to_us_since_boot(get_absolute_time());
        uint32_t _ = multicore_fifo_pop_blocking();

        st7789_start_pixels(st.pio, st.sm);
        for (int i = 0; i < 240 * 240; i++) {
            uint16_t colour = (*buffer)[i];
            st7789_lcd_put(st.pio, st.sm, colour >> 8);
            st7789_lcd_put(st.pio, st.sm, colour & 0xff);
        }
        // long time2;
        // while ((time2 = to_us_since_boot(get_absolute_time())) - time < 1000000 / framerate) {
        //     sleep_us(100);
        // }
    }
}

// TODO temp
int bl_slice_num;
void st7789_set_brightness(int brightness) {
    pwm_set_chan_level(bl_slice_num, PWM_CHAN_A, brightness);
}

#define PIN_BL 26
int main() {
    stdio_init_all();
    sleep_ms(2000);
    front_buffer = framebuffer0;
    // back_buffer = framebuffer0; // single buffering!!
    back_buffer = framebuffer1; // double buffering!!

    // st7789_init(&lcd_config, LCD_WIDTH, LCD_HEIGHT);
    // st7789_set_brightness(4);
    // st7789_fill(0x0000);

    keypad_init();

    // struct SnakeState snakestate;
    // Snake_init(&snakestate);

    MineState minestate = minesweeper_init();

    multicore_launch_core1(core1_entry);

    gpio_set_dir(PIN_BL, GPIO_OUT);
    gpio_set_function(PIN_BL, GPIO_FUNC_PWM);
    int bl_slice_num = pwm_gpio_to_slice_num(PIN_BL);
    pwm_set_wrap(bl_slice_num, 31);
    st7789_set_brightness(4);
    pwm_set_enabled(bl_slice_num, true);

    // set_sys_clock_khz(200 * 1000, true);
    // const struct mf_font_s *font = mf_find_font("fixed_10x20");
    while (1) {
        long time = to_us_since_boot(get_absolute_time());
        memset(back_buffer, 0x00, 240*240*2);
        long time1 = to_us_since_boot(get_absolute_time());
        Screen s = (Screen) {back_buffer};

        // if (font == 0) {
        //     draw_rect(s, vec2(0, 0), vec2(100, 100), 0b1111100000000000);
        // } else {
        //     draw_string_multiline(s, "Hello, world! this is a big test of the string drawing library and how far it will go. will it word wrap? not sureHello, world! this is a big test of the string drawing library and how far it will go. will it word wrap? not sureHello, world! this is a big test of the string drawing library and how far it will go. will it word wrap? not sureHello, world! this is a big test of the string drawing library and how far it will go. will it word wrap? not sureHello, world! this is a big test of the string drawing library and how far it will go. will it word wrap? not sure", vec2(0, 0), 0, font);
        // }
        // for (int i = 0; i < 240 * 240; i++) {
        //     back_buffer[i] = ii == i ? 0xffff : 0x0000;
        // }
        // ii += 1;
        // Snake_step(&snakestate, back_buffer);
        minesweeper_step(&minestate, s);
        // for (int x = 0; x < 4; x++) {
        //     for (int y = 0; y < 3; y++) {
        //         struct Key state = keypad_get(x, y);
        //         for (int dx = 0; dx < 10; dx++) {
        //             for (int dy = 0; dy < 10; dy++) {
        //                 back_buffer[x * 10 + dx + (y * 10 + dy) * 240] = state.held ? 0xFFFF : 0x2222;
        //             }
        //         }
        //     }
        // }
        // keypad_next_frame();

        // swap buffers
        uint16_t *a = front_buffer;
        front_buffer = back_buffer;
        back_buffer = a;

        long time2 = to_us_since_boot(get_absolute_time());
        multicore_fifo_push_blocking(1);
        long time3 = to_us_since_boot(get_absolute_time());
        printf("clear: %lu render: %lu fifo: %lu total: %lu\n", time1-time, time2-time1, time3-time2, time3-time);
        // spi_init(spi0, khz * 1000);
        // st7789_set_cursor(0, 0);
        // for (int i = 0; i < 240*240; i++) {
        //     st7789_put(framebuffer[i]);
        // }
        // long time2 = to_us_since_boot(get_absolute_time());
        // printf("%lu\n", time2-time);
        // printf("%d: %lu\n", khz, time2-time);
        // khz += 100;
        // draw_rect(10, 10, 0b0000000000011111);
        // st7789_fill(0xffff);
        // sleep_ms(1);
		// minesweeper();
    }
    
}