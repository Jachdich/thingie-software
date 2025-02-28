#ifndef _SNAKE_H
#define _SNAKE_H
#include <stdbool.h>
#include <stdint.h>
#include "../include/drawing.h"
#include "../include/st7789_pio.h"

#define X_BLOCKS 20
#define Y_BLOCKS 20
#define BLOCK_WIDTH (SCREEN_WIDTH / X_BLOCKS)

struct SnakeState {
    Vec2 apple;
    Vec2 v;
    Vec2 snake[X_BLOCKS * Y_BLOCKS];
    int snake_head, snake_tail;
};

void Snake_init(struct SnakeState *state);
bool Snake_step(struct SnakeState *state, Screen s);
#endif