#include "../include/drawing.h"
#include "../include/as5600.h"
#include "../include/tetris.h"
#include "../lib/st7789/st7789_pio.h"
#include "../include/keypad.h"
#include "../include/snake.h"
#include "../include/minesweeper.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// #define KB_TEST
#define PIN_BL 17
int music_step(void *, Screen);
void music_init(void *);

uint16_t framebuffer0[240*240];
uint16_t framebuffer1[240*240];
uint16_t *front_buffer;
uint16_t *back_buffer;

uint8_t *game_arena[32*1024]; // just allocate 32kiB for now...

void st7789_lcd_put(PIO a, uint b, uint8_t n);

// extern uint32_t read_ptr, write_ptr;


// #define FPM_ARM
// #define BUFFER_SIZE (1152 * 7)
// #include "../lib/libmad/mad.h"
// struct MusicState {
//     struct mad_stream stream;
//     struct mad_frame frame;
//     struct mad_synth synth;
//     float volume;
//     int16_t samples[BUFFER_SIZE];
// };


void core1_entry() {
    alarm_pool_t *pool = alarm_pool_create_with_unused_hardware_alarm(1);
    repeating_timer_t timer;
    alarm_pool_add_repeating_timer_ms(pool, 2, irq_update_keypad, NULL, &timer);
    
    uint16_t **buffer = &front_buffer;
    ST7789 st = st7789_init();
    // struct MusicState *state = (struct MusicState*)game_arena;
    while (1) {
        uint32_t _ = multicore_fifo_pop_blocking();

        st7789_start_pixels(st.pio, st.sm);
        for (int i = 0; i < 240 * 240; i++) {
            uint16_t colour = (*buffer)[i];
            st7789_lcd_put(st.pio, st.sm, colour >> 8);
            st7789_lcd_put(st.pio, st.sm, colour & 0xff);
        }

        
        // maybe limit the framerate
        // consider interrupts
    }
}

enum MainView {
    MAINVIEW_MENU,
    MAINVIEW_SNAKE,
    MAINVIEW_MINESWEEPER,
    MAINVIEW_TETRIS,
    MAINVIEW_MUSIC,
    MAINVIEW_BRIGHTNESS,
};


char bri[25] = "Brightness: 100.0%";
const char *options[] = {"Snake", "Minesweeper", "Tetris", "Music", bri};
const enum MainView states[] = {MAINVIEW_SNAKE, MAINVIEW_MINESWEEPER, MAINVIEW_TETRIS, MAINVIEW_MUSIC, MAINVIEW_BRIGHTNESS};
const int num_states = 5;

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
    // ms->view = MAINVIEW_MUSIC;
    ms->selected_item = 0;
}

float brightness = 32.0;
uint16_t wheel_tare = 0;
uint16_t last_wheel;
int16_t wheel_delta;
int16_t get_wheel_delta() {
    uint16_t wheel = as5600_read_raw_angle();
    // ((n % M) + M) % M : https://stackoverflow.com/a/1907585
    int16_t delta = ((wheel - last_wheel) % 4096 + 4096) % 4096;
    // we assume the delta is the small way around the circle.
    // if the value wraps, then delta might be very large and it's more appropriate
    // to put it in the range (-2048, 2048)
    if (delta > 2048) {
        delta -= 4096;
    }
    last_wheel = wheel;
    return delta;
}

bool on = false;
void set_on(int);

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
                break;
            case MAINVIEW_TETRIS:
                Tetris_init(game_state);
                break;
            case MAINVIEW_MUSIC: {
                // set_on(on = !on);
                // ms->view = MAINVIEW_MENU;
                // ms->view = MAINVIEW_MUSIC;
                music_init(game_state);
                break;
            }
            case MAINVIEW_BRIGHTNESS:
                // wheel_tare = as5600_read_raw_angle();
                break;
            default: break;
        }
    }

    sprintf(bri, "Brightness: %.01f%%", brightness / 64.0 * 100.0);
    
    if (states[ms->selected_item] == MAINVIEW_BRIGHTNESS) {
        brightness += wheel_delta / 4096.0 * 32.0;
        if (brightness > 64.0) brightness = 64.0;
        if (brightness < 0.0) brightness = 0.0;
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

int debug_step(void *buf_, Screen s) {
    const struct mf_font_s *font = mf_find_font("DejaVuSans12");

    uint16_t angle = as5600_read_raw_angle();
    char buf[50];
    memset(buf, 0, 50);
    sprintf(buf, "%d", as5600_read_adc());
    draw_string(s, buf, vec2(100, 100), 0xffff, font, MF_ALIGN_LEFT);
    memset(buf, 0, 50);
    sprintf(buf, "%d", angle);
    draw_string(s, buf, vec2(100, 120), 0xffff, font, MF_ALIGN_LEFT);
    memset(buf, 0, 50);

    const float step = 0.2;
    const Vec2 origin = vec2(120, 50);
    const float r = 40;
    for (float theta = 0; theta < 3.141592 * 2; theta += step) {
        Vec2 a = vec2_add(origin, vec2(r * cos(theta - step), r * sin(theta - step)));
        Vec2 b = vec2_add(origin, vec2(r * cos(theta), r * sin(theta)));
        draw_line(s, a, b, 0xffff);
    }

    float theta = (float)angle / 4096.0 * 3.1415926535 * 2.0;
    draw_line(s, origin, vec2_add(origin, vec2(r * cos(theta), r * sin(theta))), 0xffff);
    return 1;
}

struct Arena {
    void *memory;
    uint32_t pos;
    uint32_t capacity;
};

struct Arena Arena_new(void *memory, uint32_t capacity) {
    return (struct Arena){
        .memory = memory,
        .pos = 0,
        .capacity = capacity,
    };
}

void *Arena_alloc(struct Arena *a, uint32_t size) {
    if (a->pos + size > a->capacity) {
        printf("Arena overflow!!\n");
        return NULL;
    }
    void *ret = (uint8_t*)a->memory + a->pos;
    a->pos += size;
    return ret;
}

struct String {
    char *ptr;
    uint32_t len;
};

struct Song {
    struct String name;
};

struct Album {
    struct String name;
    struct Song *songs;
    uint32_t n_songs;
};

struct Artist {
    struct String name;
    struct Album *albums;
    uint32_t n_albums;
};

struct TestSong {
    char *name;
    char *album;
    char *artist;
};

void draw_box(Screen s, Vec2 pos, Vec2 size, uint16_t colour) {
    draw_xline(s, pos, size.x, colour);
    draw_xline(s, vec2_add(pos, vec2(0, size.y - 1)), size.x, colour);
    draw_yline(s, pos, size.y, colour);
    draw_yline(s, vec2_add(pos, vec2(size.x - 1, 0)), size.y, colour);
}

// int music_step(void *memory, Screen s) {
//     const struct mf_font_s *font = mf_find_font("DejaVuSans12");
//     struct Arena arena = Arena_new(memory, 10 * 1024); // TODO this is hard coded

//     const struct TestSong songs[] = {
//         (struct TestSong){"title", "album", "artist"},
//         (struct TestSong){"title2", "album2", "artist2"},
//     };

//     for (int i = 0; i < 2; i++) {
//         draw_string(s, songs[i].name, vec2(0, i * 14), 0xffff, font, MF_ALIGN_LEFT);
//     }

//     draw_string(s, "some song | some artist", vec2(4, 240 - 12 - 16), 0xffff, font, MF_ALIGN_LEFT);
//     draw_string(s, "44:30", vec2(240 - 4, 240 - 12 - 16), 0xffff, font, MF_ALIGN_RIGHT);

//     draw_box(s,  vec2(0, 240 - 10), vec2(240, 10), 0b1100011000011000);
//     draw_rect(s, vec2(1, 240 - 9),  vec2((240 - 2) / 2, 8), 0xffff);
//     return 1;
// }

void audio_test(void);

#ifdef TESTING
int main_() {
#else
int main() {
#endif
    stdio_init_all();

    // sleep_ms(3000);
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

    gpio_init(PIN_BL);
    gpio_set_dir(PIN_BL, GPIO_OUT);
    gpio_set_function(PIN_BL, GPIO_FUNC_PWM);
    int bl_slice_num = pwm_gpio_to_slice_num(PIN_BL);
    pwm_set_clkdiv(bl_slice_num, 125000000 / 4096 / 1000);
    pwm_set_wrap(bl_slice_num, 4095);
    pwm_set_enabled(bl_slice_num, true);

    
    MenuState ms;
    menu_state_init(&ms);

    // bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    i2c_init(i2c_default, 10 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    // gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    // gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    bool debug_enabled = true; 
    char debug_string[100];
    // set_sys_clock_khz(200 * 1000, true);
    set_sys_clock_khz(132000, true);


    const struct mf_font_s *mf_font_fixed_5x8 = mf_find_font("fixed_5x8");
    while (1) {
        long time = to_us_since_boot(get_absolute_time());
        memset(back_buffer, 0x00, 240*240*2);
        long time1 = to_us_since_boot(get_absolute_time());
        Screen s = (Screen) {back_buffer};
        wheel_delta = get_wheel_delta();

        #ifdef KB_TEST
        
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 3; y++) {
                struct Key state = keypad_get(x, y);
                for (int dx = 0; dx < 10; dx++) {
                    for (int dy = 0; dy < 10; dy++) {
                        back_buffer[x * 10 + dx + (y * 10 + dy) * 240] = state.held ? 0xFFFF : 0b0010000100000100;
                    }
                }
            }
        }
        keypad_next_frame();

        #else

        if (keypad_get(0, 0).held && time1 - keypad_get(0, 0).us_pressed_at > 3 * 1000 * 1000) {
            ms.view = MAINVIEW_MENU;
        }
       
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
            case MAINVIEW_TETRIS:
                if (!Tetris_step(game_state, s)) {
                    ms.view = MAINVIEW_MENU;
                }
                break;
            case MAINVIEW_MUSIC:
                if (!music_step(game_state, s)) {
                    ms.view = MAINVIEW_MENU;
                }
                break;
            case MAINVIEW_BRIGHTNESS:
                ms.view = MAINVIEW_MENU; // this can't really be clicked on
                break;
        }

        
        #endif

        if (debug_enabled) {
            draw_string(s, debug_string, vec2(0, 232), 0xffff,
                        mf_font_fixed_5x8, MF_ALIGN_LEFT);
        }

        // update brightness
        pwm_set_gpio_level(PIN_BL, (uint16_t)(brightness * brightness));

        // swap buffers
        uint16_t *a = front_buffer;
        front_buffer = back_buffer;
        back_buffer = a;

        long time2 = to_us_since_boot(get_absolute_time());
        multicore_fifo_push_blocking(1);
        long time3 = to_us_since_boot(get_absolute_time());

        if (debug_enabled) {
            memset(debug_string, 0, 100);
            sprintf(debug_string, "c:%lu r:%lu f:%lu t:%lu %f %.01f%%",
                    time1 - time, time2 - time1, time3 - time2, time3 - time,
                    1000000.0 / (float)(time3 - time),
                    (time2 - time1) / (10000.0 / 60.0));
        }
    }

    #ifdef TESTING
    sleep_ms(11);
    #endif
    
    
}
