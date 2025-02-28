#include "../include/drawing.h"
#include "../include/snake.h"
#include "../include/st7789_pio.h"
#include "../include/keypad.h"
#include <stdlib.h>
#include "pico/time.h"

void Snake_new_apple(struct SnakeState *state) {
    // while (true) {
        state->apple_x = rand() % X_BLOCKS;
        state->apple_y = rand() % Y_BLOCKS;
        // for (int i = state->snake_tail; i != state->snake_head; i = (i + 1) % (X_BLOCKS * Y_BLOCKS)) {
        //     int x = state->snake[i] & 0xFF;
        //     int y = (state->snake[i] >> 8) & 0xFF;
        //     if (x == state->apple_x && y == state->apple_y) {
                
        //     }
        // }
    // }
}

void Snake_init(struct SnakeState *state) {
    Snake_new_apple(state);
    state->vx = 1;
    state->vy = 0;
    state->snake_head = 1;
    state->snake_tail = 0;
    state->snake[0] = (2 << 8) | 2;
}

// TODO
// buffer 2 inputs
// if, after executing the first in the control buffer, the second command is a no-op; then delete it to make room for more inputs

void Snake_update_velocity(struct SnakeState *state) {
    if (state->vy == 0) {
        if (keypad_get(2, 0).pressed) {
            state->vy = -1;
            state->vx = 0;
            // state->input_this_frame = true;
        }
        if (keypad_get(2, 2).pressed) {
            state->vy = 1;
            state->vx = 0;
            // state->input_this_frame = true;
        }
    } else if (state->vx == 0) {
        if (keypad_get(1, 1).pressed) {
            state->vy = 0;
            state->vx = -1;
            // state->input_this_frame = true;
        }
        if (keypad_get(3, 1).pressed) {
            state->vy = 0;
            state->vx = 1;
            // state->input_this_frame = true;
        }
    }
}

inline int wrap(int i) {
    while (i < 0) i += X_BLOCKS * Y_BLOCKS;
    return i % (X_BLOCKS * Y_BLOCKS);
}

void Snake_step(struct SnakeState *state, Screen s) {
    long time = to_ms_since_boot(get_absolute_time());
    Snake_update_velocity(state);
    keypad_next_frame();
    int head_x = state->snake[wrap(state->snake_head - 1)] & 0xFF;
    int head_y = (state->snake[wrap(state->snake_head - 1)] >> 8) & 0xFF;
    head_x += state->vx;
    head_y += state->vy;
    if (head_x < X_BLOCKS && head_x >= 0 && head_y < X_BLOCKS && head_y >= 0) {
        state->snake[state->snake_head] = (head_y << 8) | head_x;
        state->snake_head = wrap(state->snake_head + 1);
        if (head_x == state->apple_x && head_y == state->apple_y) {
            Snake_new_apple(state);
        } else {
             state->snake_tail = wrap(state->snake_tail + 1);
        }
    }

    // for (int x = 0; x < X_BLOCKS; x++) {
    //     for (int y = 0; y < Y_BLOCKS; y++) {
    //         int col;
    //         switch (state->blocks[y * X_BLOCKS + x]) {
    //             case BLOCK_SNAKE: col = 0b0000011111100000; break;
    //             case BLOCK_EMPTY: col = 0x39c7; break;
    //             case BLOCK_APPLE: col = 0b1111100000000000; break;
    //             case BLOCK_DONE:  col = 0b0000000000011111; break;
    //         }
    //         draw_rect(x * BLOCK_WIDTH, y * BLOCK_WIDTH, col);
    //     }
    // }

    // st7789_fill(0x0000);
    // int i = wrap(state->snake_tail - 1);
    // int x = state->snake[i] & 0xFF;
    // int y = (state->snake[i] >> 8) & 0xFF;
    // draw_rect(x * BLOCK_WIDTH, y * BLOCK_WIDTH, 0b0000000000000000, buffer);
    // if ((state->snake_tail - state->snake_head) % (X_BLOCKS * Y_BLOCKS) != 1) {
    //     int x2 = state->snake[state->snake_tail] & 0xFF;
    //     int y2 = (state->snake[state->snake_tail] >> 8) & 0xFF;
    //     int vx = x - x2;
    //     int vy = y - y2;
    //     draw_rect(x * BLOCK_WIDTH - vx * 4, y * BLOCK_WIDTH - vy * 4, 0, buffer);
    // }

    for (int i = state->snake_tail; i != state->snake_head; i = (i + 1) % (X_BLOCKS * Y_BLOCKS)) {
        int x = state->snake[i] & 0xFF;
        int y = (state->snake[i] >> 8) & 0xFF;

        if ((i + 1) % (X_BLOCKS * Y_BLOCKS) != state->snake_head) {
            draw_rect(s, vec2(x * BLOCK_WIDTH, y * BLOCK_WIDTH), vec2(8, 8), 0b0000011111100000);
        } else {
            draw_rect(s, vec2(x * BLOCK_WIDTH, y * BLOCK_WIDTH), vec2(8, 8), 0b1100111111100000);
        }
        if ((i + 1) % (X_BLOCKS * Y_BLOCKS) != state->snake_head) {
            int x2 = state->snake[(i + 1) % (X_BLOCKS * Y_BLOCKS)] & 0xFF;
            int y2 = (state->snake[(i + 1) % (X_BLOCKS * Y_BLOCKS)] >> 8) & 0xFF;
            draw_rect(s, vec2((x+x2) * BLOCK_WIDTH / 2, (y + y2) * BLOCK_WIDTH / 2), vec2(8, 8), 0b0000011000000000);
        }
    }
    draw_rect(s, vec2(state->apple_x * BLOCK_WIDTH, state->apple_y * BLOCK_WIDTH), vec2(8, 8), 0b1111100000000000);

    long time2;
    while ((time2 = to_ms_since_boot(get_absolute_time())) - time < (1000.0 / 6.0)) {
        sleep_ms(1);
    }
}

