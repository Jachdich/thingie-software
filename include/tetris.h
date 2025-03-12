#ifndef _TETRIS_H
#define _TETRIS_H
#include "../include/drawing.h"
#include <stdbool.h>

struct TetrisState;
bool Tetris_step(struct TetrisState *state, Screen s);
void Tetris_init(struct TetrisState *state);
#endif