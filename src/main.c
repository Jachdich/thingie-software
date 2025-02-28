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

uint8_t *game_arena[10*1024]; // just allocate 10kiB for now...

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

enum MainView {
    MAINVIEW_MENU,
    MAINVIEW_SNAKE,
    MAINVIEW_MINESWEEPER,
    MAINVIEW_TETRIS,
};


const char *options[] = {"Snake", "Minesweeper", "Tetris"};
const enum MainView states[] = {MAINVIEW_SNAKE, MAINVIEW_MINESWEEPER, MAINVIEW_TETRIS};
const int num_states = 3;

typedef struct {
    enum MainView view;
    int selected_item;
    const struct mf_font_s *font;
    Vec2 pos;
    Vec2 size;
} MenuState;

void menu_state_init(MenuState *ms) {
    int max_width = 0;
    ms->font = mf_find_font("DejaVuSans12");
    for (int i = 0; i < num_states; i++) {
        int w = mf_get_string_width(ms->font, options[i], 0, true);
        if (w > max_width) {
            max_width = w;
        }
    }
    max_width += 20; // 10 for > indicator, 10 for border
    ms->pos.x = (240 - max_width) / 2;
    ms->pos.y = 50;
    ms->size.x = max_width;
    ms->size.y = 12 * num_states + 10;
    ms->view = MAINVIEW_MENU;
    ms->selected_item = 0;
}


void draw_menu(Screen s, void *game_state, MenuState *ms) {
    if (keypad_get(2, 0).pressed) {
        ms->selected_item -= 1;
        if (ms->selected_item < 0) ms->selected_item = 0;
    }
    if (keypad_get(2, 2).pressed) {
        ms->selected_item += 1;
        if (ms->selected_item >= num_states) ms->selected_item = num_states - 1;
    }
    if (keypad_get(2, 1).pressed) {
        ms->view = states[ms->selected_item];
        switch (ms->view) {
            case MAINVIEW_SNAKE:
                Snake_init(game_state);
                break;
            case MAINVIEW_MINESWEEPER:
                minesweeper_init(game_state);
            default: break;
        }
    }
    keypad_next_frame();

    draw_xline(s, ms->pos, ms->size.x, 0xffff);
    draw_xline(s, vec2_add(vec2(0, ms->size.y), ms->pos), ms->size.x, 0xffff);
    draw_yline(s, ms->pos, ms->size.y, 0xffff);
    draw_yline(s, vec2_add(vec2(ms->size.x, 0), ms->pos), ms->size.y, 0xffff);
    for (int i = 0; i < num_states; i++) {
        draw_string(s, options[i], vec2_add(vec2(15, 12 * i + 5), ms->pos), 0xffff, ms->font, MF_ALIGN_LEFT);
    }
    draw_string(s, ">", vec2_add(vec2(5, 12 * ms->selected_item + 5), ms->pos), 0xffff, ms->font, MF_ALIGN_LEFT);
}


#define PIN_BL 26
int main() {
    stdio_init_all();

    // Initialise keypad + read manually once, to check whether to wait (for programming)
    keypad_init();
    irq_update_keypad(NULL); 
    if (keypad_get(0, 0).held) {
        sleep_ms(3000);
    }
    front_buffer = framebuffer0;
    // back_buffer = framebuffer0; // single buffering!!
    back_buffer = framebuffer1; // double buffering!!

    void *game_state = game_arena;

    multicore_launch_core1(core1_entry);

    gpio_set_dir(PIN_BL, GPIO_OUT);
    gpio_set_function(PIN_BL, GPIO_FUNC_PWM);
    int bl_slice_num = pwm_gpio_to_slice_num(PIN_BL);
    pwm_set_wrap(bl_slice_num, 31);
    st7789_set_brightness(4);
    pwm_set_enabled(bl_slice_num, true);

    MenuState ms;
    menu_state_init(&ms);

    // set_sys_clock_khz(200 * 1000, true);
    // const struct mf_font_s *font = mf_find_font("fixed_10x20");
    while (1) {
        long time = to_us_since_boot(get_absolute_time());
        memset(back_buffer, 0x00, 240*240*2);
        long time1 = to_us_since_boot(get_absolute_time());
        Screen s = (Screen) {back_buffer};

        switch (ms.view) {
            case MAINVIEW_MENU:
                draw_menu(s, game_state, &ms);
                break;
            case MAINVIEW_SNAKE:
                if (!Snake_step(game_state, s)) {
                    ms.view = MAINVIEW_MENU;
                }
                break;
            case MAINVIEW_MINESWEEPER:
                if (!minesweeper_step(game_state, s)) {
                    ms.view = MAINVIEW_MENU;
                }
                break;
        }

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
        // minesweeper_step(game_state, s);
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