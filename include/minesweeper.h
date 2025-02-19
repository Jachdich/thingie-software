#ifndef _MINESWEEPER_H
#define _MINESWEEPER_H
#include <stdint.h>
typedef struct {
    uint8_t *board;
    int selected_x, selected_y;
    int width, height, cell_width, cell_height;
    int num_mines;
} GameState;
void minesweeper_step(GameState *state, uint16_t *buffer);
GameState minesweeper_init(uint8_t *board, int w, int h);
#endif
