//
// Created by ignacy on 12/01/2026.
//

#include "../include/scroller.h"
#include "../include/keypad.h"
#include "../include/drawing.h"
#include "../lib/mcufont/mcufont.h"
#include "pico/time.h"
#include <stdlib.h>
#include <stdio.h>

/* ================= CONFIG ================= */

#define TARGET_FPS 60
#define FRAME_TIME_MS (1000 / TARGET_FPS)

#define GRAVITY 0.5
#define JUMP_VEL -8

#define COL_BG          0b0100101000101011
#define COL_RED         0b0111000000000000
#define COL_WHITE       0b1111111111011111
#define COL_LIGHT_GRAY  0b1110011001011001
#define COL_GRAY        0b0110101101001101
#define COL_BLACK       0b0000000000000000
#define COL_NONE        0b0000100000100001

static const struct mf_font_s *font;
char score_buf[32];
char high_buf[32];
char time_buf[32];

/* ================== PLAYER ================== */

#define PLAYER_W   16
#define PLAYER_H   18

static uint8_t player_pixels[PLAYER_W * PLAYER_H] = {
    0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 9, 8, 8, 8, 8, 9, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 9, 8, 8, 8, 8, 9, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 9, 8, 8, 8, 8, 9, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static uint16_t player_palette[] = {COL_NONE,
    0b1111110000000000,  // outline
    0b0000011111100000,  // body
    0,0,0,0,0, COL_WHITE, COL_BLACK
};

static uint16_t player_palette_alt[] = {COL_NONE,
    COL_WHITE,  // outline
    COL_WHITE,  // body
    0,0,0,0,0, COL_WHITE, COL_BLACK
};

static PaletteImage PLAYER_IMG = {
    .data = player_pixels,
    .size = {PLAYER_W, PLAYER_H},
    .palette = player_palette
};

static PaletteImage PLAYER_IMG_ALT = {
    .data = player_pixels,
    .size = {PLAYER_W, PLAYER_H},
    .palette = player_palette_alt
};


/* ================== TILES ================== */
/* --- GROUND --- */
static uint8_t ground_pixels[TILE_SIZE * TILE_SIZE] = {
    0, 4, 4, 4, 4, 4, 0, 4, 0, 4, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
    4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
    4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
    4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
    4, 3, 3, 2, 2, 2, 3, 3, 2, 2, 1, 2, 3, 3, 2, 2, 2, 3, 3, 3, 
    2, 2, 2, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 2, 2, 2, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static uint16_t ground_palette[] = {COL_NONE,
    0b1011110000101010, // dirt body
    0b1000001010100101, // dirt edge
    0b0100010000100101, // grass body
    0b0110011000101000, // grass edge
    0,0,COL_WHITE, COL_BLACK
};

static PaletteImage TILE_GROUND_IMG = {
    .data = ground_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = ground_palette
};

/* --- DIRT --- */
static uint8_t dirt_pixels[TILE_SIZE * TILE_SIZE] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static PaletteImage TILE_DIRT_IMG = {
    .data = dirt_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = ground_palette
};

/* --- BLOCK + PLATFORM --- */
static uint8_t block_pixels[TILE_SIZE * TILE_SIZE] = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 
    9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 
    9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9
};

static uint8_t platform_pixels[TILE_SIZE * TILE_SIZE] = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 
    9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 
    9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 
    9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 
    9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 
    9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 
    9, 2, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 1, 1, 1, 1, 1, 9, 
    9, 2, 1, 1, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 1, 1, 1, 9, 
    9, 2, 1, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 1, 1, 9, 
    9, 2, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 1, 9, 
    9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static uint8_t breakable_pixels[TILE_SIZE * TILE_SIZE] = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 
    9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 
    9, 2, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 9, 
    9, 2, 1, 2, 2, 2, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 1, 1, 1, 2, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 1, 2, 2, 1, 2, 2, 2, 2, 1, 1, 1, 2, 1, 1, 1, 2, 2, 9, 
    9, 2, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 2, 1, 2, 9, 
    9, 2, 1, 2, 2, 1, 1, 1, 2, 1, 2, 1, 2, 1, 1, 2, 1, 1, 2, 9, 
    9, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 1, 1, 2, 2, 1, 1, 2, 9, 
    9, 2, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 2, 2, 1, 2, 1, 1, 2, 9, 
    9, 2, 2, 2, 2, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 2, 2, 2, 9, 
    9, 2, 1, 1, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2, 1, 1, 1, 1, 2, 9, 
    9, 2, 1, 2, 2, 1, 1, 2, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 2, 9, 
    9, 2, 2, 1, 2, 1, 2, 1, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 9, 
    9, 2, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 9, 
    9, 2, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 1, 2, 2, 2, 1, 1, 2, 9, 
    9, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 2, 9, 
    9, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 2, 9, 
    9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 9, 
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9
};

static uint16_t block_palette[] = {COL_NONE,
    0b1001001111001100,  // body
    0b0111101100001001,  // edge
    0,0,0,0,COL_WHITE, COL_BLACK
};

static PaletteImage TILE_BLOCK_IMG = {
    .data = block_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = block_palette
};

static PaletteImage TILE_PLATFORM_IMG = {
    .data = platform_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = block_palette
};

static PaletteImage TILE_BREAKABLE_IMG = {
    .data = breakable_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = block_palette
};

static uint16_t bg_block_palette[] = {COL_NONE,
    0b0100000111000101,  // body
    0b0101001000000110,  // edge
    0,0,0,0,COL_WHITE, COL_BLACK
};

static PaletteImage TILE_BG_BLOCK_IMG = {
    .data = block_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = bg_block_palette
};

/* --- COIN --- */
static uint8_t coin_pixels[TILE_SIZE * TILE_SIZE] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 9, 9, 1, 1, 1, 1, 1, 1, 9, 9, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 9, 1, 1, 3, 3, 3, 3, 3, 3, 1, 1, 9, 0, 0, 0, 0, 
    0, 0, 0, 9, 1, 3, 3, 1, 1, 1, 1, 1, 1, 3, 2, 1, 9, 0, 0, 0, 
    0, 0, 0, 9, 1, 3, 1, 1, 3, 3, 3, 2, 1, 1, 2, 1, 9, 0, 0, 0, 
    0, 0, 9, 1, 3, 1, 1, 1, 3, 1, 1, 2, 1, 1, 1, 2, 1, 9, 0, 0, 
    0, 0, 9, 1, 3, 1, 1, 1, 3, 1, 1, 2, 1, 1, 1, 2, 1, 9, 0, 0, 
    0, 0, 9, 1, 3, 1, 1, 1, 3, 1, 1, 2, 1, 1, 1, 2, 1, 9, 0, 0, 
    0, 0, 9, 1, 3, 1, 1, 1, 3, 1, 1, 2, 1, 1, 1, 2, 1, 9, 0, 0, 
    0, 0, 9, 1, 3, 1, 1, 1, 3, 1, 1, 2, 1, 1, 1, 2, 1, 9, 0, 0, 
    0, 0, 9, 1, 3, 1, 1, 1, 3, 1, 1, 2, 1, 1, 1, 2, 1, 9, 0, 0, 
    0, 0, 0, 9, 1, 3, 1, 1, 3, 2, 2, 2, 1, 1, 2, 1, 9, 0, 0, 0, 
    0, 0, 0, 9, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 1, 9, 0, 0, 0, 
    0, 0, 0, 0, 9, 1, 1, 2, 2, 2, 2, 2, 2, 1, 1, 9, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 9, 9, 1, 1, 1, 1, 1, 1, 9, 9, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static uint16_t coin_palette[] = {COL_NONE,
    0b1111011101000000,  // body
    0b1111111001100101,  // dark
    COL_WHITE,  // light
    0,0,0,COL_WHITE, COL_BLACK
};

static PaletteImage TILE_COIN_IMG = {
    .data = coin_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = coin_palette
};

/* --- HEART --- */
static uint8_t heart_pixels[TILE_SIZE * TILE_SIZE] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 9, 9, 9, 0, 0, 0, 0, 0, 0, 9, 9, 9, 0, 0, 0, 0, 
    0, 0, 0, 9, 2, 2, 2, 9, 0, 0, 0, 0, 9, 2, 2, 2, 9, 0, 0, 0, 
    0, 0, 9, 2, 3, 3, 3, 2, 9, 0, 0, 9, 2, 3, 3, 3, 2, 9, 0, 0, 
    0, 9, 2, 3, 3, 1, 1, 1, 2, 9, 9, 2, 3, 3, 1, 1, 1, 2, 9, 0, 
    9, 2, 3, 3, 1, 1, 1, 1, 2, 9, 9, 2, 3, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 3, 1, 1, 1, 1, 1, 1, 2, 2, 3, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    9, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 
    0, 9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 0, 
    0, 9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 2, 9, 0, 
    0, 0, 9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 2, 9, 0, 0, 
    0, 0, 9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 2, 9, 0, 0, 
    0, 0, 0, 9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 9, 0, 0, 0, 
    0, 0, 0, 0, 9, 2, 1, 1, 1, 1, 1, 3, 1, 1, 2, 9, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 9, 2, 1, 1, 1, 1, 1, 1, 2, 9, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 9, 2, 1, 1, 1, 1, 2, 9, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 9, 2, 1, 1, 2, 9, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 9, 2, 2, 9, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static uint16_t heart_palette[] = {COL_NONE,
    0b1111000110100110,  // body
    0b1110001110001110,  // dark
    COL_WHITE,  // light
    0,0,0,COL_WHITE, COL_BLACK
};

static PaletteImage TILE_HEART_IMG = {
    .data = heart_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = heart_palette
};
/* --- SPIKE --- */
static uint8_t spike_pixels[TILE_SIZE * TILE_SIZE] = {
    9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 
    9, 2, 2, 9, 0, 9, 9, 9, 0, 3, 3, 0, 9, 9, 9, 0, 9, 2, 2, 9, 
    9, 2, 8, 2, 9, 2, 2, 9, 0, 3, 3, 0, 9, 2, 2, 9, 2, 8, 2, 9, 
    0, 9, 2, 1, 2, 8, 2, 9, 3, 4, 4, 3, 9, 2, 8, 2, 1, 2, 9, 0, 
    0, 0, 9, 2, 1, 2, 9, 3, 9, 4, 4, 9, 3, 9, 2, 1, 2, 9, 0, 0, 
    0, 9, 2, 8, 2, 1, 2, 9, 2, 9, 9, 2, 9, 2, 1, 2, 8, 2, 9, 0, 
    0, 9, 2, 2, 9, 2, 1, 2, 9, 4, 4, 9, 2, 1, 2, 9, 2, 2, 9, 0, 
    0, 9, 9, 9, 3, 9, 2, 1, 2, 9, 9, 2, 1, 2, 9, 3, 9, 9, 9, 0, 
    0, 0, 0, 3, 9, 2, 9, 2, 1, 2, 2, 1, 2, 9, 2, 9, 3, 0, 0, 0, 
    0, 3, 3, 4, 4, 9, 4, 9, 2, 1, 1, 2, 9, 4, 9, 4, 4, 3, 3, 0, 
    0, 3, 3, 4, 4, 9, 4, 9, 2, 1, 1, 2, 9, 4, 9, 4, 4, 3, 3, 0, 
    0, 0, 0, 3, 9, 2, 9, 2, 1, 2, 2, 1, 2, 9, 2, 9, 3, 0, 0, 0, 
    0, 9, 9, 9, 3, 9, 2, 1, 2, 9, 9, 2, 1, 2, 9, 3, 9, 9, 9, 0, 
    0, 9, 2, 2, 9, 2, 1, 2, 9, 4, 4, 9, 2, 1, 2, 9, 2, 2, 9, 0, 
    0, 9, 2, 8, 2, 1, 2, 9, 2, 9, 9, 2, 9, 2, 1, 2, 8, 2, 9, 0, 
    0, 0, 9, 2, 1, 2, 9, 3, 9, 4, 4, 9, 3, 9, 2, 1, 2, 9, 0, 0, 
    0, 9, 2, 1, 2, 8, 2, 9, 3, 4, 4, 3, 9, 2, 8, 2, 1, 2, 9, 0, 
    9, 2, 8, 2, 9, 2, 2, 9, 9, 3, 3, 9, 9, 2, 2, 9, 2, 8, 2, 9, 
    9, 2, 2, 9, 0, 9, 9, 9, 9, 3, 3, 9, 9, 9, 9, 0, 9, 2, 2, 9, 
    9, 9, 9, 0, 0, 0, 0, 0, 0, 9, 9, 0, 0, 0, 0, 0, 0, 9, 9, 9
};

static uint16_t spike_palette[] = {COL_NONE,
    COL_WHITE,  // body
    0b1110001110001110,  // dark
    COL_WHITE,  // light
    0,0,0,COL_WHITE, COL_BLACK
};

static PaletteImage TILE_SPIKE_IMG = {
    .data = spike_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = spike_palette
};

static PaletteImage *tile_images[] = {
    [TILE_EMPTY]     = NULL,
    [TILE_GROUND]    = &TILE_GROUND_IMG,
    [TILE_DIRT]      = &TILE_DIRT_IMG,
    [TILE_BLOCK]     = &TILE_BLOCK_IMG,
    [TILE_PLATFORM]  = &TILE_PLATFORM_IMG,
    [TILE_BG_BLOCK]  = &TILE_BG_BLOCK_IMG,
    [TILE_BREAKABLE] = &TILE_BREAKABLE_IMG,
    [TILE_COIN]      = &TILE_COIN_IMG,
    [TILE_HEART]     = &TILE_HEART_IMG,
    [TILE_SPIKE]     = &TILE_SPIKE_IMG
};

/* ================= SECTIONS ================= */
static const uint16_t sections[][MAP_H][MAP_W] = {
    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
    }, {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 8, 0, 0, 0, 0, 6, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 4, 0, 0, 0, 0, 3, 0, 0, 0}, 
        {0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
    }, {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 4, 6, 6, 6, 4, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 4, 0, 7, 0, 4, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 4, 0, 3, 0, 4, 0, 0},
        {0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 5, 0, 7, 0, 5, 0, 0}, 
        {1, 1, 4, 3, 3, 3, 4, 3, 3, 3, 4, 1, 1}, 
        {2, 2, 4, 0, 0, 0, 4, 0, 0, 0, 4, 2, 2}, 
        {2, 2, 4, 0, 0, 0, 4, 0, 0, 0, 4, 2, 2}
    }, {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0},
        {0, 0, 0, 4, 3, 3, 3, 3, 3, 4, 0, 0, 0}, 
        {0, 0, 0, 5, 0, 0, 7, 0, 0, 5, 0, 0, 0},
        {0, 3, 0, 5, 0, 7, 7, 7, 0, 5, 0, 3, 0}, 
        {0, 0, 0, 5, 0, 0, 7, 0, 0, 5, 0, 0, 0},
        {1, 1, 1, 4, 3, 0, 0, 0, 3, 4, 1, 1, 1},
        {2, 2, 2, 4, 0, 0, 3, 0, 0, 4, 2, 2, 2},
        {2, 2, 2, 4, 0, 0, 0, 0, 0, 4, 2, 2, 2}
    }, {
        {0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 3, 0, 0, 0, 8, 0, 0, 0, 3, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
    }, {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 4, 0, 4, 0, 4, 0, 0, 0}, 
        {0, 0, 0, 0, 4, 4, 0, 4, 0, 4, 4, 0, 0}, 
        {0, 0, 0, 4, 4, 4, 0, 4, 0, 4, 4, 4, 0}, 
        {0, 0, 4, 4, 4, 4, 7, 4, 0, 4, 4, 4, 4}, 
        {1, 1, 1, 1, 1, 1, 7, 1, 8, 1, 1, 1, 1},
        {2, 2, 2, 2, 2, 2, 7, 2, 0, 2, 2, 2, 2}, 
        {2, 2, 2, 2, 2, 2, 0, 2, 0, 2, 2, 2, 2}
    }, {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 7, 0, 4, 3, 3, 3, 3, 3, 4, 0, 0, 0}, 
        {0, 0, 0, 4, 0, 7, 7, 7, 7, 4, 0, 0, 0}, 
        {0, 3, 3, 4, 3, 4, 4, 6, 4, 4, 0, 0, 0},
        {0, 0, 0, 4, 0, 4, 0, 5, 0, 5, 0, 0, 0}, 
        {0, 0, 0, 4, 0, 5, 0, 4, 0, 5, 0, 0, 0}, 
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
    }
};

#define SECTION_COUNT (sizeof(sections) / sizeof(sections[0]))

static int current_section;
static int section_col;
static float scroll_px;

/* =========== SKIES ========== */
static inline bool needs_stars(enum SkyType sky) {
    switch (sky) {
        case SKY_NIGHT:
        case SKY_MIDNIGHT:
            return true;
        default:
            return false;
    }
}

static const uint16_t SKY_NIGHT_COLS[] = {
    0b0000100010000101, // deep space blue
    0b0001000100001010, // night sky blue
    0b0010001000010000, // moonlight blue
    0b0011001100011010, // early dawn blue
    0b0100010000011100, // soft horizon glow
    0b0110010100011010  // warm dawn haze
};

static const uint16_t SKY_DAWN_COLS[] = {
    0b0010001000010000,
    0b0011001100011010,
    0b0100010000011100,
    0b0110010100011010,
};

/* ============= DRAWING HELPERS ============= */
static const uint8_t PARALLAX_SPEED[3] = {
    2,    // far
    4,    // mid
    7    // near
};

static inline int brightness_565(uint16_t c) {
    int r = (c >> 11) & 0x1F;
    int g = (c >> 5)  & 0x3F;
    int b =  c        & 0x1F;
    return r * 2 + g * 3 + b * 2; // cheap weighted brightness
}

static void move_stars(struct ScrollerState *s, Screen screen) {
    long now = to_ms_since_boot(get_absolute_time());
    if (s->frame_start == 0)
        s->frame_start = now;
    float dt = (now - s->frame_start) / 1000.0f;
    s->frame_start = now; // reset

    float scroll_px = dt * s->scroll_speed * TILE_SIZE; // float, pixels per frame

    for (int i = 0; i < MAX_STARS; i++) {
        Star *star = &s->stars[i];

        star->frac += star->speed;
        if (star->frac >= 10) {
            int dx = star->frac / 10;
            star->frac %= 10;
            star->x -= dx;
        }

        // recycle if fully off-screen
        if (star->x + star->size < 0) {
            star->x = SCREEN_W + star->size;
            star->y = rand() % (SCREEN_H - INFO_H - TILE_SIZE) + INFO_H;

            star->size = rand() % 5;
            star->layer = star->size % 3;
            star->speed = PARALLAX_SPEED[star->layer];
            star->frac = rand() % 10;
			star->flicker_timer = 0;
            star->off = false;
            continue;
        }

        star->flicker_timer += dt;   // dt in milliseconds
        if (!star->off && star->flicker_timer > 800 + rand() % 800) {
            star->off = true;
            star->flicker_timer = 0;
        }
        else if (star->off && star->flicker_timer > 100 + rand() % 200) {
            star->off = false;
            star->flicker_timer = 0;
        }

    }
}

void draw_stars(struct ScrollerState *s, Screen screen) {
    for (int i = 0; i < MAX_STARS; i++) {
        Star *star = &s->stars[i];
        int size = star->layer;

        uint16_t sky = get_px(screen, vec2(star->x, star->y));
        int b = brightness_565(sky);
        if (b >= 120)
            continue;
        if (star->off)
            continue;
        uint16_t col;
        if (b < 30)
            col = COL_WHITE;
        else if (b < 70)
            col = COL_LIGHT_GRAY;
        else
            col = COL_GRAY;

        if (size <= 0 || size > 3) {
            draw_px(screen, vec2(star->x, star->y), col);
        } else {
            draw_yline(screen, vec2(star->x, star->y - size), size * 2 + 1, col);
            draw_xline(screen, vec2(star->x - size, star->y), size * 2 + 1, col);
        }
    }
}


/* ============= DRAWING =========== */
void draw_sky_night(Screen screen) {
    Vec2 center = vec2(SCREEN_W / 2, SCREEN_H - INFO_H);
    int radius = 170;
    for (int i = 0; i < sizeof(SKY_NIGHT_COLS)/sizeof(*SKY_NIGHT_COLS); i++) {
        draw_circle(screen, center, radius, SKY_NIGHT_COLS[i]);
        radius -= 35;
    }
}

void draw_sky_midnight(Screen screen) {
    Vec2 center = vec2(SCREEN_W / 2, SCREEN_H - INFO_H);
    draw_circle(screen, center, 200, SKY_NIGHT_COLS[0]);
}

void draw_sky_dawn(Screen s) {
    Vec2 c = vec2(SCREEN_W / 2, INFO_H + 180);
    int r = 300;
    for (int i = 0; i < 4; i++, r -= 50)
        draw_circle(s, c, r, SKY_DAWN_COLS[i]);
}

void draw_sky_gradient(Screen s) {
    uint16_t col1 = SKY_NIGHT_COLS[1];
    uint16_t col2 = COL_BG;
    draw_gradient(s, vec2(0, INFO_H), vec2(SCREEN_W, SCREEN_H - INFO_H), col1, col2, UP_RIGHT);
}

void draw_bg(struct ScrollerState *s, Screen screen) {
    switch (s->sky) {
        case SKY_NIGHT:       draw_sky_night(screen); draw_stars(s, screen); break;
        case SKY_MIDNIGHT:    draw_sky_midnight(screen); draw_stars(s, screen); break;
        case SKY_DAWN:        draw_sky_dawn(screen); break;
        case SKY_GRADIENT:    draw_sky_gradient(screen); break;
        default: return;
    }
}

/* ================= HELPERS ================= */
static enum Tile get_tile(struct ScrollerState *s, int px, int py) {
    int tx = (px + (int)scroll_px) / TILE_SIZE;
    int ty = (py - INFO_H) / TILE_SIZE;

    if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H)
        return TILE_EMPTY;

    return s->map[ty][tx];
}

static void set_tile(struct ScrollerState *s, int px, int py, enum Tile t) {
    int tx = (px + (int)scroll_px) / TILE_SIZE;
    int ty = (py - INFO_H) / TILE_SIZE;

    if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H)
        return;

    s->map[ty][tx] = t;
}

static void reset_time(struct ScrollerState *s) {
    s->frame_start = 0;
    s->last_time_ms = to_ms_since_boot(get_absolute_time());
    s->time_accum = 0.0f;
}

static void handle_death(struct ScrollerState *s) {
    snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);
    snprintf(high_buf, sizeof(high_buf), "H.Score: %d", s->highscore);
    snprintf(time_buf, sizeof(time_buf), "Time: %d", s->seconds_alive);
	if (s->score > s->highscore) {
		s->highscore = s->score;
		s->view = SCROLLER_VIEW_NEW_SCORE;
	} else {
		s->view = SCROLLER_VIEW_GAME_OVER;
	}
}

static void handle_reset(struct ScrollerState *s) {
	s->player.pos = vec2f(40, INFO_H + 100);
    s->player.vel = vec2f(0, 0);
    s->player.on_ground = false;
    s->player.lives = 1;
    s->player.invulnerable = 0;

	current_section = 1;
    section_col = 0;
    scroll_px = 0.0f;

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            s->map[y][x] = sections[current_section][y][x];
        }
    }

    s->sky = rand() % SKY_COUNT;
    // s->sky = SKY_GRADIENT;

    if (needs_stars(s->sky)) {
        for (int i = 0; i < MAX_STARS; i++) {
            Star *star = &s->stars[i];

            star->x = rand() % SCREEN_W;
            star->y = rand() % (SCREEN_H - INFO_H - TILE_SIZE) + INFO_H;

            star->size = rand() % 5;               // farther = smaller
            star->layer = star->size % 3;
            star->speed = PARALLAX_SPEED[star->layer];
            star->frac = rand() % 10;
            star->flicker_timer = 0;
            star->off = false;
        }
    }

	s->score = 0;
    s->seconds_alive = 0;
    s->scroll_timer = 0.0f;
    s->scroll_speed = 0.8f;
    s->player.move_speed = 2.0f;

    reset_time(s);

    snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);
    snprintf(high_buf, sizeof(high_buf), "H.Score: %d", s->highscore);
    snprintf(time_buf, sizeof(time_buf), "Time: %d", s->seconds_alive);
}

static void damage(struct ScrollerState *s, int dmg) {
    if (s->player.lives - dmg >= 0) {
        s->player.lives -= dmg;
        s->player.invulnerable = 3;
    }
    if (s->player.lives <= 0) handle_death(s);
}

/* ================= Collision Helpers ================= */

// Block or ground → fully solid
bool solid_full(enum Tile t) {
    return t == TILE_BLOCK || t == TILE_GROUND || t == TILE_BREAKABLE || t == TILE_SPIKE;
}

// Only solid when landing from above
bool solid_top(enum Tile t) {
    return solid_full(t) || t == TILE_PLATFORM;
}

// Only block sides
bool solid_side(enum Tile t) {
    return solid_full(t);
}

/* ================= MAP STREAMING ================= */

static void shift_map_left(struct ScrollerState *s) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W - 1; x++)
            s->map[y][x] = s->map[y][x + 1];

        s->map[y][MAP_W - 1] =
            sections[current_section][y][section_col];
    }

    section_col++;
    if (section_col >= MAP_W) {
        section_col = 0;
        int next_section = current_section;
        while (next_section == current_section) {
            next_section = (rand() % (SECTION_COUNT - 1)) + 1;
        }
        current_section = next_section;
    }
}

/* ================= INIT ================= */
void Scroller_init(struct ScrollerState *s) {
    printf("init");
	s->selected = 0;
	s->view = SCROLLER_VIEW_PLAYING;

    s->highscore = 0;
	handle_reset(s);

    font = mf_find_font("DejaVuSans12");
}

/* ================= STEPS ================= */
bool Scroller_paused(struct ScrollerState *s, Screen screen) {
    long now = to_ms_since_boot(get_absolute_time());

    if (keypad_get(3,0).pressed || keypad_get(2,1).pressed) {
        reset_time(s);
        s->view = SCROLLER_VIEW_PLAYING;
    }
    keypad_next_frame();

    draw_bg(s, screen);

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            PaletteImage *img = tile_images[sections[0][y][x]];
            if (!img) continue;
            draw_palette(screen, *img,
                vec2(
                    x * TILE_SIZE - (int)scroll_px,
                    INFO_H + y * TILE_SIZE
                )
            );
        }
    }

    draw_string(screen, "PAUSED", vec2(120, 25), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, high_buf, vec2(120, 60), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, score_buf, vec2(120, 74), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, time_buf, vec2(120, 110), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, "Continue", vec2(120, 140), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, "[", vec2(90, 140), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, "]", vec2(150, 140), COL_WHITE, font, MF_ALIGN_CENTER);

    long frame_end = to_ms_since_boot(get_absolute_time());
    long frame_time = frame_end - now;

    if (frame_time < FRAME_TIME_MS) {
        sleep_ms(FRAME_TIME_MS - frame_time);
    }

    return true;
}

bool Scroller_endgame(struct ScrollerState *s, Screen screen, const char *message, const struct mf_font_s *font) {
    bool go = keypad_get(2, 1).pressed;
    if (keypad_get(2, 0).pressed) {
        s->selected -= 1;
        if (s->selected < 0) s->selected = 1;
    }
    if (keypad_get(2, 2).pressed) {
        s->selected += 1;
        if (s->selected > 1) s->selected = 0;
    }
    keypad_next_frame();
    draw_rect(screen, vec2(0, 0), vec2(SCREEN_W, SCREEN_H), 0b0011000010100010);
    draw_string(screen, message, vec2(120, 25), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, high_buf, vec2(120, 60), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, score_buf, vec2(120, 74), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, time_buf, vec2(120, 110), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, "Again", vec2(120, 140), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, "Quit", vec2(120, 156), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, "[", vec2(100 + 5 * s->selected, 140 + 16 * s->selected), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, "]", vec2(140 - 4 * s->selected, 140 + 16 * s->selected), COL_WHITE, font, MF_ALIGN_CENTER);

    if (s->selected == 0 && go) {
        handle_reset(s);
        s->view = SCROLLER_VIEW_PLAYING;
    }
    if (s->selected == 1 && go) return false;
    return true;
}

bool Scroller_main(struct ScrollerState *s, Screen screen) {
    /* -------- TIME -------- */
    long now = to_ms_since_boot(get_absolute_time());
    if (s->frame_start == 0)
        s->frame_start = now;

    float dt = (now - s->frame_start) / 1000.0f;
    s->frame_start = now;
    s->time_accum += dt;

    /* -------- SCROLL -------- */
    scroll_px += dt * s->scroll_speed * TILE_SIZE;
    s->player.pos.x -= dt * s->scroll_speed * TILE_SIZE;

    while (scroll_px >= TILE_SIZE) {
        scroll_px -= TILE_SIZE;
        shift_map_left(s);
    }

    if (s->time_accum >= 1.0f) {
        s->time_accum -= 1.0f;
        s->seconds_alive++;
        s->score += 5;

        if (s->seconds_alive % 60 == 0)
            s->score += 50;

        s->scroll_speed += 0.03f;
        s->player.move_speed += 0.03f;

        snprintf(time_buf, sizeof(time_buf), "Time: %d", s->seconds_alive);
        snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);

        if (s->player.invulnerable > 0) {
            s->player.invulnerable -= 1;
        }
    }
    bool invulnerable = s->player.invulnerable > 0;

    /* -------- INPUT -------- */
    if (keypad_get(3,0).pressed) {
        reset_time(s);
        s->view = SCROLLER_VIEW_PAUSED;
    }
    s->player.vel.x *= 0.8;
    if (keypad_get(1,1).held) s->player.vel.x = -s->player.move_speed;
    if (keypad_get(3,1).held) s->player.vel.x = s->player.move_speed;
    
    if (keypad_get(2,0).pressed && s->player.on_ground) {
        s->player.vel.y = JUMP_VEL;
        s->player.on_ground = false;
    }
    keypad_next_frame();

    /* -------- PHYSICS -------- */
    s->player.vel.y += GRAVITY;
    Vec2f next = s->player.pos;

    /* --- X AXIS (side collisions) --- */
    next.x += s->player.vel.x;

    enum Tile s1a = get_tile(s, next.x + PLAYER_W - 1, next.y);
    enum Tile s1b = get_tile(s, next.x + PLAYER_W - 1, next.y + PLAYER_H - 1);
    enum Tile s2a = get_tile(s, next.x, next.y);
    enum Tile s2b = get_tile(s, next.x, next.y + PLAYER_H - 1);
    if (s->player.vel.x >= 0) { // moving right
        if (solid_side(s1a) || solid_side(s1b)) {
            next.x = (((int)next.x + PLAYER_W - 1 + (int)scroll_px) / TILE_SIZE) * TILE_SIZE - PLAYER_W - (int)scroll_px;
            if ((s1a == TILE_SPIKE || s1b == TILE_SPIKE) && !invulnerable) damage(s, 1);
        }
    } else if (s->player.vel.x < 0) { // moving left
        if (solid_side(s2a) || solid_side(s2b)) {
            next.x = (((int)next.x + (int)scroll_px) / TILE_SIZE + 1) * TILE_SIZE - (int)scroll_px;
            if ((s2a == TILE_SPIKE || s2b == TILE_SPIKE) && !invulnerable) damage(s, 1);
        }
    }

    /* --- RIGHT EDGE COLLISIONS --- */
    if (next.x > SCREEN_WIDTH - PLAYER_W)
        next.x = SCREEN_WIDTH - PLAYER_W;

    /* --- HORIZONTAL DEATH --- */
    if (next.x + 1 < 0)
        handle_death(s);

    /* --- Y AXIS --- */
    next.y += s->player.vel.y;

    // Sample player bottom
    enum Tile b1 = get_tile(s, next.x + 2, next.y + PLAYER_H);
    enum Tile b2 = get_tile(s, next.x + PLAYER_W - 2, next.y + PLAYER_H);

    // Collision when falling onto solid top (platform or ground)
    if ((solid_top(b1) || solid_top(b2)) && s->player.vel.y >= 0) {
        // Align bottom of player to top of tile
        int tile_y = ((int)(next.y + PLAYER_H) / TILE_SIZE) * TILE_SIZE;
        next.y = tile_y - PLAYER_H;

        s->player.vel.y = 0;
        s->player.on_ground = true;

        if ((b1 == TILE_SPIKE || b2 == TILE_SPIKE) && !invulnerable) {
            damage(s, 1);
        }
    }
    else {
        s->player.on_ground = false;
    }

    // Ceiling collision (jumping into blocks)
    enum Tile t1 = get_tile(s, next.x + 2, next.y);
    enum Tile t2 = get_tile(s, next.x + PLAYER_W - 2, next.y);
    if (((solid_full(t1) || solid_full(t2)) && s->player.vel.y < 0) || next.y < INFO_H + 1) {
        next.y = (((int)next.y) / TILE_SIZE + 1) * TILE_SIZE;
        s->player.vel.y = 0;
        if ((t1 == TILE_SPIKE || t2 == TILE_SPIKE) && !invulnerable) {
            damage(s, 1);
        }
    }
    s->player.pos = next;

    /* --- Breakables --- */
    if ((t1 == TILE_BREAKABLE || t2 == TILE_BREAKABLE) && s->player.lives>1) {
        set_tile(s, next.x + PLAYER_W / 2, next.y - 2, TILE_EMPTY);
        s->score += 1;
        snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);
    }

    /* --- COINS --- */
    if (get_tile(s, next.x + PLAYER_W / 2, next.y + PLAYER_H / 2) == TILE_COIN) {
        set_tile(s, next.x + PLAYER_W / 2, next.y + PLAYER_H / 2, TILE_EMPTY);
        s->score += 100;
        snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);
    }

    /* --- HEARTS --- */
    if (get_tile(s, next.x + PLAYER_W / 2, next.y + PLAYER_H / 2) == TILE_HEART && s->player.lives < 3) {
        set_tile(s, next.x + PLAYER_W / 2, next.y + PLAYER_H / 2, TILE_EMPTY);
        s->player.lives += 1;
        snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);
    }

    /* -------- VERTICAL DEATH -------- */
    if (s->player.pos.y > SCREEN_HEIGHT)
		handle_death(s);

    /* -------- DRAW -------- */
    if (needs_stars(s->sky)) move_stars(s, screen);
    draw_bg(s, screen);

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            PaletteImage *img = tile_images[s->map[y][x]];
            if (!img) continue;
            draw_palette(screen, *img,
                vec2(
                    x * TILE_SIZE - (int)scroll_px,
                    INFO_H + y * TILE_SIZE
                )
            );
        }
    }

    draw_rect(screen, vec2(0, INFO_H), vec2(2, SCREEN_H - INFO_H), COL_RED);
	draw_yline(screen, vec2(0, 0), INFO_H, COL_WHITE);
	draw_yline(screen, vec2(SCREEN_W - 1, 0), INFO_H, COL_WHITE);
    draw_xline(screen, vec2(0, INFO_H), SCREEN_W, COL_WHITE);
    draw_xline(screen, vec2(0, 0), SCREEN_W, COL_WHITE);

    if (invulnerable && (s->player.invulnerable == 1 || s->time_accum < 0.1 || (0.6 < s->time_accum && s->time_accum < 0.7))) draw_palette(screen, PLAYER_IMG_ALT, vec2f_to_vec2(s->player.pos));
    else draw_palette(screen, PLAYER_IMG, vec2f_to_vec2(s->player.pos));

    for (int i = 0; i < s->player.lives; i++) {
        Vec2 pos = vec2(2 + 22 * i, INFO_H + 3);
        draw_palette(screen, TILE_HEART_IMG, pos);
    }

    draw_string(screen, score_buf, vec2(2,3), COL_WHITE, font, MF_ALIGN_LEFT);
    draw_string(screen, time_buf, vec2(-2,3), COL_WHITE, font, MF_ALIGN_RIGHT);

    long frame_end = to_ms_since_boot(get_absolute_time());
    long frame_time = frame_end - now;

    return true;
}

bool Scroller_step(struct ScrollerState *s, Screen screen) {
    if (keypad_get(0,0).f_pressed > 160) return false;
	switch (s->view) {
	    case SCROLLER_VIEW_PAUSED:
	        return Scroller_paused(s, screen);
		case SCROLLER_VIEW_PLAYING:
			return Scroller_main(s, screen);
		case SCROLLER_VIEW_NEW_SCORE:
			return Scroller_endgame(s, screen, "New High Score!", font);
		case SCROLLER_VIEW_GAME_OVER:
		    return Scroller_endgame(s, screen, "Game Over :(", font);
		default:
            keypad_next_frame();
            break;
	}
	return true;
}
