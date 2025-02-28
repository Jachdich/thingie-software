#ifndef _MINESWEEPER_H
#define _MINESWEEPER_H
#include <stdint.h>
#include "../include/drawing.h"

typedef enum {
    MINEVIEW_INIT_MENU,
    MINEVIEW_GAME_OVER,
    MINEVIEW_COMPLETE,
    MINEVIEW_PLAYING
} Mine_View;

typedef struct {
    uint8_t board[26*26];
    int selected_x, selected_y;
    int width, height, cell_rad;
    int num_mines;
    Mine_View view;
} MineState;
void minesweeper_step(MineState *state, Screen s);
MineState minesweeper_init();
#endif
