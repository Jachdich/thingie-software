#include "pico/time.h"
#include "../include/keypad.h"
#include "../include/drawing.h"
#include <string.h>
#include <stdbool.h>

#define X_BLOCKS 10
#define Y_BLOCKS 20
#define BLOCK_SIZE 12

typedef enum {
    EMPTY = 0xFF, I = 0, J = 1, L = 2, Z = 3, S = 4, O = 5, T = 6,
} Piece;

uint8_t shape[] = {
    0, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0, 
    1, 1, 1, 1,  0, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0, 
    0, 0, 0, 0,  0, 0, 1, 0,  1, 1, 1, 1,  0, 1, 0, 0, 
    0, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0, 

    1, 0, 0, 0,  0, 1, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0,
    1, 1, 1, 0,  0, 1, 0, 0,  1, 1, 1, 0,  0, 1, 0, 0,
    0, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  1, 1, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

    0, 0, 1, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1, 1, 0, 0,
    1, 1, 1, 0,  0, 1, 0, 0,  1, 1, 1, 0,  0, 1, 0, 0,
    0, 0, 0, 0,  0, 1, 1, 0,  1, 0, 0, 0,  0, 1, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

    1, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0,
    0, 1, 1, 0,  0, 1, 1, 0,  1, 1, 0, 0,  1, 1, 0, 0,
    0, 0, 0, 0,  0, 1, 0, 0,  0, 1, 1, 0,  1, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

    0, 1, 1, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,
    1, 1, 0, 0,  0, 1, 1, 0,  0, 1, 1, 0,  1, 1, 0, 0,
    0, 0, 0, 0,  0, 0, 1, 0,  1, 1, 0, 0,  0, 1, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

    0, 1, 1, 0,  0, 1, 1, 0,  0, 1, 1, 0,  0, 1, 1, 0,
    0, 1, 1, 0,  0, 1, 1, 0,  0, 1, 1, 0,  0, 1, 1, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

    0, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,
    1, 1, 1, 0,  0, 1, 1, 0,  1, 1, 1, 0,  1, 1, 0, 0,
    0, 0, 0, 0,  0, 1, 0, 0,  0, 1, 0, 0,  0, 1, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
};

uint16_t piece_colours[] = {
    0b0000011110011110,
    0b0000000000011110,
    0b1111110111000000,
    0b1111100000000000,
    0b0000011111100000,
    0b1111011110000000,
    0b1110000000011100,
};

typedef struct {
    Piece blocks[X_BLOCKS * Y_BLOCKS];
    Piece current_piece;
    uint8_t rotation;
    Vec2 pos;
    long last_move_time;
} TetrisState;

Vec2 vec2_mul(Vec2 v, int x) {
    return vec2(v.x * x, v.y * x);
}

int get_index_from_piece(Piece p, uint8_t rotation, Vec2 pos) {
    return 4 * 4 * 4 * p + 4 * 4 * pos.y + 4 * rotation + pos.x;
}

void draw_piece(Screen s, Vec2 pos, Vec2 topleft, Piece p, uint8_t rotation) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int idx = get_index_from_piece(p, rotation, vec2(x, y));
            if (shape[idx]) {
                Vec2 rect_pos = vec2_add(pos, vec2(x, y));
                draw_rect(s, vec2_add(topleft, vec2_mul(rect_pos, BLOCK_SIZE)), vec2(BLOCK_SIZE, BLOCK_SIZE), piece_colours[p]);
            }
        }
    }
}

Vec2 width(Piece p, uint8_t rotation) {
    // Return the horizontal extent of a piece in this rotation.
    // The x component of the resultant vector is the left side of the piece
    // and the y componnet is the right side.
    Vec2 max = vec2(100, 0);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int idx = get_index_from_piece(p, rotation, vec2(x, y));
            if (shape[idx]) {
                if (x < max.x) max.x = x;
                if (x > max.y) max.y = x;
            }
        }
    }
    return max;
}

void Tetris_init(TetrisState *state) {
    memset(state->blocks, EMPTY, X_BLOCKS * Y_BLOCKS);
    state->current_piece = I;
    state->rotation = 0;
    state->pos = vec2(0, 0);
    state->last_move_time = to_ms_since_boot(get_absolute_time());
}

bool collide(Piece p, Vec2 pos, int rotation, Piece *blocks) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int idx = get_index_from_piece(p, rotation, vec2(x, y));
            if (shape[idx] && y + 1 >= Y_BLOCKS) {
                return true;
            }
            Vec2 global_pos = vec2_add(pos, vec2(x, y + 1));
            if (shape[idx] && blocks[global_pos.y * X_BLOCKS + global_pos.x] != EMPTY) {
                return true;
            }
        }
    }
    return false;
}

void add_to_background(Piece p, Vec2 pos, int rotation, Piece *blocks) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int idx = get_index_from_piece(p, rotation, vec2(x, y));
            Vec2 global_pos = vec2_add(pos, vec2(x, y));
            if (shape[idx]) {
                blocks[global_pos.y * X_BLOCKS + global_pos.x] = p;
            }
        }
    }
}

void move_down(TetrisState *state) {
    state->pos.y += 1;
    if (collide(state->current_piece, state->pos, state->rotation, state->blocks)) {
        add_to_background(state->current_piece, state->pos, state->rotation, state->blocks);
        state->pos = vec2(X_BLOCKS / 2 - 1, 0);
        state->current_piece = rand() % 7; // TODO pick from a bag
        state->rotation = 0;
    }
}

void try_rotate(TetrisState *state, int amount) {
    state->rotation = (state->rotation + amount) % 4;
}

bool Tetris_step(TetrisState *state, Screen s) {
    if (keypad_get(2, 1).pressed) {
        try_rotate(state, 1);
    }
    Vec2 w = width(state->current_piece, state->rotation);
    if (keypad_get(3, 1).pressed && w.y + state->pos.x + 1 < X_BLOCKS) {
        state->pos.x += 1;
    }
    if (keypad_get(1, 1).pressed && w.x + state->pos.x - 1 >= 0) {
        state->pos.x -= 1;
    }
    keypad_next_frame();
    long curr_time = to_ms_since_boot(get_absolute_time());
    if (curr_time - state->last_move_time > 300) {
        state->last_move_time = curr_time;
        move_down(state);
    }
    Vec2 board_start = vec2(60, 0);
    draw_yline(s, vec2_add(board_start, vec2(-1, 0)), Y_BLOCKS * BLOCK_SIZE, 0xFFFF);
    draw_yline(s, vec2_add(board_start, vec2(X_BLOCKS * BLOCK_SIZE + 1, 0)), Y_BLOCKS * BLOCK_SIZE, 0xFFFF);
    // draw_yline(s, vec2)
    draw_piece(s, state->pos, board_start, state->current_piece, state->rotation);
    for (int y = 0; y < Y_BLOCKS; y++) {
        for (int x = 0; x < X_BLOCKS; x++) {
            Piece block = state->blocks[y * X_BLOCKS + x];
            if (block != EMPTY) {
                draw_rect(s, vec2_add(board_start, vec2_mul(vec2(x, y), BLOCK_SIZE)), vec2(BLOCK_SIZE, BLOCK_SIZE), piece_colours[block]);
            }
        }
    }
    return true;
}