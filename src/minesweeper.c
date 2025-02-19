#include "../include/keypad.h"
// #include "../include/st7789.h"
#include "../include/minesweeper.h"
#include <stdint.h>
#include <stdlib.h>
#include "pico/time.h"

//TODO
//chording
//main menu
//better scaling
//num of mines
//proper flag
//mines left display
//time
//fix annoying "clever" code

// void print(const char *n);

#define MINE 0x0F

const uint16_t neighbour_colours[] = {
    0b0000000000000000,
    0b0000000000011111,
    0b0000010110000000,
    0b1111100000000000,
    0b0000100000011000,
    0b1000000000000000,
    0b0000010000010000,
    0b0000000000000000,
    0b1000010000010000,
};

static unsigned char big_numbers[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,0,0,0,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,
	0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};
static unsigned char big_revealed_data[] = {
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};
static unsigned char big_unrevealed_data[] = {
    2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    2,3,3,3,3,3,3,3,3,3,3,3,3,3,1,
    2,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    2,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    2,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,3,4,4,4,4,4,4,4,4,4,4,4,1,1,
    3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static unsigned char small_revealed_data[] = {
	7,7,7,7,7,7,7,7,7,
	7,6,6,6,6,6,6,6,7,
	7,6,6,6,6,6,6,6,7,
	7,6,6,6,6,6,6,6,7,
	7,6,6,6,6,6,6,6,7,
	7,6,6,6,6,6,6,6,7,
	7,6,6,6,6,6,6,6,7,
	7,6,6,6,6,6,6,6,7,
	7,7,7,7,7,7,7,7,7
};

static unsigned char small_unrevealed_data[] = {
	2,2,2,2,2,2,2,2,1,
	2,2,2,2,2,2,2,1,1,
	2,2,3,3,3,3,3,1,1,
	2,2,3,3,3,3,3,1,1,
	2,2,3,3,3,3,3,1,1,
	2,2,3,3,3,3,3,1,1,
	2,2,3,3,3,3,3,1,1,
	2,2,1,1,1,1,1,1,1,
	2,1,1,1,1,1,1,1,1
	};

// TODO bit pack these?
uint8_t small_numbers[] = {
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,1,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,0,0,0,0,1,0,0,
	0,0,0,0,1,1,0,0,0,
	0,0,0,1,0,0,0,0,0,
	0,0,1,0,0,0,0,0,0,
	0,0,1,1,1,1,1,0,0,
	0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,0,0,0,0,1,0,0,
	0,0,0,0,1,1,0,0,0,
	0,0,0,0,0,0,1,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,
	
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,1,0,0,0,
	0,0,0,1,0,1,0,0,0,
	0,0,1,0,0,1,0,0,0,
	0,0,1,1,1,1,1,0,0,
	0,0,0,0,0,1,0,0,0,
	0,0,0,0,0,1,0,0,0,
	0,0,0,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,
	0,0,0,1,1,1,1,0,0,
	0,0,1,0,0,0,0,0,0,
	0,0,1,0,0,0,0,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,0,0,0,0,1,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,1,0,0,0,
	0,0,0,1,0,0,0,0,0,
	0,0,1,0,0,0,0,0,0,
	0,0,1,1,1,1,0,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,
	0,0,1,1,1,1,0,0,0,
	0,0,0,0,0,0,1,0,0,
	0,0,0,0,0,0,1,0,0,
	0,0,0,0,0,1,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,1,0,0,0,0,0,
	0,0,0,1,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,1,0,0,0,1,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,
	
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,0,1,0,0,0,0,
	0,0,0,1,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,
	0,0,0,1,0,0,0,0,0,
	0,0,1,1,0,0,0,0,0,
	0,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0.
};

struct Image {
	uint8_t *data;
	int size;
};

struct Image numbers[] = {
	(struct Image){.data = small_numbers + 9*9*0, .size = 9},
	(struct Image){.data = small_numbers + 9*9*1, .size = 9},
	(struct Image){.data = small_numbers + 9*9*2, .size = 9},
	(struct Image){.data = small_numbers + 9*9*3, .size = 9},
	(struct Image){.data = small_numbers + 9*9*4, .size = 9},
	(struct Image){.data = small_numbers + 9*9*5, .size = 9},
	(struct Image){.data = small_numbers + 9*9*6, .size = 9},
	(struct Image){.data = small_numbers + 9*9*7, .size = 9},
	(struct Image){.data = small_numbers + 9*9*8, .size = 9},
	(struct Image){.data = small_numbers + 9*9*9, .size = 9},
	(struct Image){.data = big_numbers + 15*15*0, .size = 15},
	(struct Image){.data = big_numbers + 15*15*1, .size = 15},
	(struct Image){.data = big_numbers + 15*15*2, .size = 15},
	(struct Image){.data = big_numbers + 15*15*3, .size = 15},
	(struct Image){.data = big_numbers + 15*15*4, .size = 15},
	(struct Image){.data = big_numbers + 15*15*5, .size = 15},
	(struct Image){.data = big_numbers + 15*15*6, .size = 15},
	(struct Image){.data = big_numbers + 15*15*7, .size = 15},
	(struct Image){.data = big_numbers + 15*15*8, .size = 15},
	(struct Image){.data = big_numbers + 15*15*9, .size = 15},
};

struct Image small_unrevealed = (struct Image){.data = small_unrevealed_data, .size = 9};
struct Image small_revealed   = (struct Image){.data = small_revealed_data, .size = 9};
struct Image big_unrevealed   = (struct Image){.data = big_unrevealed_data, .size = 15};
struct Image big_revealed     = (struct Image){.data = big_revealed_data, .size = 15};

int cmap[] = {0x0000, 0x630c, 0x7bcf, 0x7bef, 0x9492, 0xb5b6, 0xffff, 0xdefb};

int count_neighbours(GameState *state, int x, int y) {
    int neighbours = 0;
    for (int dx = -1; dx < 2; dx++) {
        for (int dy = -1; dy < 2; dy++) {
            int bx = x + dx;
            int by = y + dy;
            if (bx < 0 || bx >= state->width ||
                by < 0 || by >= state->height) {
                continue;
            }
            if (state->board[by * state->width + bx] == MINE) {
                neighbours += 1;
            }
        }
    }
    return neighbours;
}

void generate_board(GameState *state) {
    for (int i = 0; i < state->num_mines; i++) {
        int pos;
        //make sure the chosen pos doesn't already have a mine
        do {
            pos = rand() % (state->width * state->height);
        } while (state->board[pos] == MINE);
        state->board[pos] = MINE;
    }

    //number non-mine squares
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            if (state->board[y * state->width + x] == MINE) {
                continue;
            }
            state->board[y * state->width + x] = count_neighbours(state, x, y);
        }
    }
}

//return whether the game should continue
bool reveal(GameState *state, int x, int y) {
    if (state->board[y * state->width + x] & 0x80) {
        //already revealed
        return true;
    }
    if (state->board[y * state->width + x] & 0x40) {
        //flagged, don't reveal
        return true;
    }
    if (state->board[y * state->width + x] == MINE) {
        //explode
        return false;
    }
    
    state->board[y * state->width + x] |= 0x80;
    if (state->board[y * state->width + x] == 0x80) {
        //0 neighbours, recursively propagate
        for (int dx = -1; dx < 2; dx++) {
            for (int dy = -1; dy < 2; dy++) {
                if (x + dx < 0 || x + dx >= state->width ||
                    y + dy < 0 || y + dy >= state->height) {
                    continue;
                }
                if (state->board[(y + dy) * state->width + x + dx] & 0x80) {
                    continue;
                }
                
                reveal(state, x + dx, y + dy);
            }
        }
    }
    
    return true;
}

void draw_transparent(struct Image img, int x, int y, int colour, uint16_t *buffer) {
    for (int i = 0; i < img.size; i++) {
        for (int j = 0; j < img.size; j++) {
            if (x + j >= 240 || y + i >= 240) {
                printf("Out of range (transparent) %d %d %d %d %d\n", x, j, y, i, img.size);
            }
			if (img.data[i * img.size + j]) {
    			buffer[x + j + (y + i) * 240] = colour;
			}
		}
	}
}
void draw_coloured(struct Image img, int x, int y, int *cmap, uint16_t *buffer) {
    for (int i = 0; i < img.size; i++) {
        for (int j = 0; j < img.size; j++) {
            if (x + j >= 240 || y + i >= 240) {
                printf("Out of range (coloured) %d %d %d %d\n", x, j, y, i);
            }
			unsigned char px = img.data[i * img.size + j];
			buffer[x + j + (y + i) * 240] = cmap[px];
        }
    }
}

void draw_tile(int x, int y, GameState *state, uint16_t *buffer) {
    uint8_t tile = state->board[y * state->width + x];
    x = x * state->cell_width;// + 1;
    y = y * state->cell_height;// + 1;
    int w = state->cell_width - 2;
    int h = state->cell_height - 2;
    
    uint16_t revealed_col = 0b1100011000011000;
    uint16_t unrevealed_col = 0b0111001110001110;
    const int flagpole_colour = 0; // black
    const int flag_colour = 0b1111100000000000; // red
    
    bool big = state->cell_width == 15 ? 1 : 0; // TODO hacky

    if (tile & 0x80) {
        //revealed
        uint8_t neighbours = tile & 0x0F;
        uint16_t col = neighbour_colours[neighbours];

        // tft->fillRect(x, y, w, h, revealed_col);
        draw_coloured(big ? big_revealed : small_revealed, x, y, cmap, buffer);
        if (neighbours > 0) {
            draw_transparent(numbers[(neighbours - 1) + 10 * big], x, y, neighbour_colours[neighbours], buffer);
        }
    } else if (tile & 0x40) {
        //flagged
        // tft->fillRect(x, y, w, h, unrevealed_col);
        // draw_centred_char(tft, '!', 0b1111100000000000, unrevealed_col, x, y, w, h, state);
        draw_coloured(big ? big_unrevealed : small_unrevealed, x, y, cmap, buffer);
        draw_transparent(numbers[8 + 10 * big], x, y, flagpole_colour, buffer);
        draw_transparent(numbers[9 + 10 * big], x, y, flag_colour, buffer);
    } else {
        // tft->fillRect(x, y, w, h, unrevealed_col);
        draw_coloured(big ? big_unrevealed : small_unrevealed, x, y, cmap, buffer);
    }
}

void draw_xline(uint16_t *buffer, int x0, int x1, int y, uint16_t col) {
    for (int x = x0; x < x1; x++) {
        buffer[y * 240 + x] = col;
    }
}

void draw_yline(uint16_t *buffer, int x, int y0, int y1, uint16_t col) {
    for (int y = y0; y < y1; y++) {
        buffer[y * 240 + x] = col;
    }
}

void draw_selected(uint16_t *buffer, int x0, int y0, int w, int h, uint16_t col) {
    draw_xline(buffer, x0, x0 + w, y0, col);
    draw_xline(buffer, x0, x0 + w, y0 + h - 1, col);
    if (y0 - 1 >= 0) {
        draw_xline(buffer, x0, x0 + w, y0 - 1, col);
    }
    if (y0 + h < 240) {
        draw_xline(buffer, x0, x0 + w, y0 + h, col);
    }

    draw_yline(buffer, x0, y0, y0 + h, col);
    draw_yline(buffer, x0 + w - 1, y0, y0 + h, col);

    if (x0 - 1 >= 0) {
        draw_yline(buffer, x0 - 1, y0, y0 + h, col);
    }
    if (x0 + w < 240) {
        draw_yline(buffer, x0 + w, y0, y0 + h, col);
    }
}

void draw(GameState *state, uint16_t *buffer) {
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            draw_tile(x, y, state, buffer);
            // uint16_t border = state->selected_x == x && state->selected_y == y ? 0b1111111111111111 : 0b1000010000010000;
            // tft->drawRect(x * state->cell_width, y * state->cell_height, state->cell_width, state->cell_height, border);
        }
    }
    draw_selected(buffer, state->selected_x * state->cell_width,
                          state->selected_y * state->cell_height,
                          state->cell_width, state->cell_height,
                          0xb8b1);
}

bool check_win(GameState *state) {
    int unrevealed = 0;
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            //if the top bit (i.e. revealed) isn't set
            if (!((state->board[y * state->width + x] & 0x80) >> 7)) {
                unrevealed += 1;
            }
        }
    }
    return unrevealed == state->num_mines;
}

GameState minesweeper_init(uint8_t *board, int width, int height) {
    //semi random ish, read from uninitialised analog pin
    // srand(analogRead(A7) ^ micros());
    srand(to_us_since_boot(get_absolute_time()));
    
    //board: mine = 0x0F, non-mine = n where n is the number of neighbours
    //top bit set if revealed (|= 0x80)
    //2nd-top bit set if flagged (|= 0x40)
    memset(board, 0, width * height);

    int cell_rad = width > 16 ? 9 : 15;

    GameState state = {
        board,
        0, 0,
        width, height, cell_rad, cell_rad,
        width * height / 6
    };
    
    generate_board(&state);
    return state;
}

void minesweeper_step(GameState *state, uint16_t *buffer) {
    state->selected_x += (keypad_get(3, 1).pressed) && state->selected_x < state->width - 1;
    state->selected_y += (keypad_get(2, 2).pressed) && state->selected_y < state->height - 1;
    state->selected_x -= (keypad_get(1, 1).pressed) && state->selected_x > 0;
    state->selected_y -= (keypad_get(2, 0).pressed) && state->selected_y > 0;
    if (keypad_get(2, 1).released) {
     if (!reveal(state, state->selected_x, state->selected_y)) {
         // print("Game over\n");
         return;
     } else if (check_win(state)) {
         // print("Success\n");
         return;
     }
    }
    if (keypad_get(0, 1).released) {
     //toggle flagging, if unrevealed
     int idx = state->selected_y * state->width + state->selected_x;
     if (!(state->board[idx] & 0x80)) {
         state->board[idx] ^= 0x40;
     }
    }
    keypad_next_frame();
    draw(state, buffer);
}
