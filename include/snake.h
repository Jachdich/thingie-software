#ifndef _SNAKE_H
#define _SNAKE_H
#include <stdbool.h>
#include <stdint.h>
#include "../include/st7789_pio.h"

#define X_BLOCKS 20
#define Y_BLOCKS 20
#define BLOCK_WIDTH (SCREEN_WIDTH / X_BLOCKS)

struct SnakeState {
    int apple_x, apple_y;
    int vx, vy;
    int snake[X_BLOCKS * Y_BLOCKS];
    int snake_head, snake_tail;
};

void Snake_init(struct SnakeState *state);
void Snake_step(struct SnakeState *state, uint16_t *buffer);
#endif