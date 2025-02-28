#include "../include/drawing.h"
#include "../include/snake.h"
#include "../include/st7789_pio.h"
#include "../include/keypad.h"
#include <stdlib.h>
#include "pico/time.h"

void Snake_new_apple(struct SnakeState *state) {
    // while (true) {
        state->apple.x = rand() % X_BLOCKS;
        state->apple.y = rand() % Y_BLOCKS;
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
    state->v = vec2(1, 0);
    state->snake_head = 1;
    state->snake_tail = 0;
    state->snake[0] = vec2(2, 2);
}

// TODO
// buffer 2 inputs
// if, after executing the first in the control buffer, the second command is a no-op; then delete it to make room for more inputs

void Snake_update_velocity(struct SnakeState *state) {
    if (state->v.y == 0) {
        if (keypad_get(2, 0).pressed) {
            state->v.y = -1;
            state->v.x = 0;
            // state->input_this_frame = true;
        }
        if (keypad_get(2, 2).pressed) {
            state->v.y = 1;
            state->v.x = 0;
            // state->input_this_frame = true;
        }
    } else if (state->v.x == 0) {
        if (keypad_get(1, 1).pressed) {
            state->v.y = 0;
            state->v.x = -1;
            // state->input_this_frame = true;
        }
        if (keypad_get(3, 1).pressed) {
            state->v.y = 0;
            state->v.x = 1;
            // state->input_this_frame = true;
        }
    }
}

inline int wrap(int i) {
    while (i < 0) i += X_BLOCKS * Y_BLOCKS;
    return i % (X_BLOCKS * Y_BLOCKS);
}
inline bool vec2_eq(Vec2 a, Vec2 b) {
    return a.x == b.x && a.y == b.y;
}

bool intersect_snake(struct SnakeState *state) {
    Vec2 head = state->snake[wrap(state->snake_head - 1)];
    for (int i = state->snake_tail; i != wrap(state->snake_head - 1); i = wrap(i + 1)) {
        if (vec2_eq(state->snake[i], head)) return true;
    }
    return false;
}

bool Snake_step(struct SnakeState *state, Screen s) {
    long time = to_ms_since_boot(get_absolute_time());
    Snake_update_velocity(state);
    keypad_next_frame();
    Vec2 head = state->snake[wrap(state->snake_head - 1)];
    head.x += state->v.x;
    head.y += state->v.y;
    if (head.x < X_BLOCKS && head.x >= 0 && head.y < X_BLOCKS && head.y >= 0) {
        state->snake[state->snake_head] = head;
        state->snake_head = wrap(state->snake_head + 1);
        if (head.x == state->apple.x && head.y == state->apple.y) {
            Snake_new_apple(state);
        } else {
             state->snake_tail = wrap(state->snake_tail + 1);
        }
    } else {
        return false;
    }

    if (intersect_snake(state)) {
        return false;
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
        Vec2 pos = state->snake[i];

        if ((i + 1) % (X_BLOCKS * Y_BLOCKS) != state->snake_head) {
            draw_rect(s, vec2(pos.x * BLOCK_WIDTH, pos.y * BLOCK_WIDTH), vec2(8, 8), 0b0000011111100000);
        } else {
            draw_rect(s, vec2(pos.x * BLOCK_WIDTH, pos.y * BLOCK_WIDTH), vec2(8, 8), 0b1100111111100000);
        }
        if ((i + 1) % (X_BLOCKS * Y_BLOCKS) != state->snake_head) {
            Vec2 pos2 = vec2_add(pos, state->snake[(i + 1) % (X_BLOCKS * Y_BLOCKS)]);
            draw_rect(s, vec2(pos2.x * BLOCK_WIDTH / 2, pos2.y * BLOCK_WIDTH / 2), vec2(8, 8), 0b0000011000000000);
        }
    }
    draw_rect(s, vec2(state->apple.x * BLOCK_WIDTH, state->apple.y * BLOCK_WIDTH), vec2(8, 8), 0b1111100000000000);

    long time2;
    while ((time2 = to_ms_since_boot(get_absolute_time())) - time < (1000.0 / 6.0)) {
        sleep_ms(1);
    }
    return true;
}

