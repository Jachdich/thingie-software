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

#define GRAVITY 1
#define JUMP_VEL -11
#define MOVE_SPEED 2

#define COL_BG      0b0100101000101011
#define COL_RED     0b0111000000000000
#define COL_WHITE   0b1111111111011111
#define COL_BLACK   0b0000000000000000
#define COL_NONE    0b0000100000100001

static const struct mf_font_s *font;
char score_buf[32];
char high_buf[32];
char time_buf[32];

/* ================== PLAYER ================== */

#define PLAYER_W   16
#define PLAYER_H   18

static uint8_t player_pixels[PLAYER_W * PLAYER_H] = {
    0,0,0,0,0,0,9,9,9,9,0,0,0,0,0,0,
    0,0,0,0,0,9,8,8,8,8,9,0,0,0,0,0,
    0,0,0,0,0,9,8,8,8,8,9,0,0,0,0,0,
    0,0,0,0,0,9,8,8,8,8,9,0,0,0,0,0,
    0,0,0,0,0,0,9,9,9,9,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,
    0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,
    0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,
    0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,
    0,0,0,0,0,2,2,0,0,2,2,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static uint16_t player_palette[] = {COL_NONE,
    0b1111110000000000,  // outline
    0b0000011111100000,  // body
    0,0,0,0,0, COL_WHITE, COL_BLACK
};

static PaletteImage PLAYER_IMG = {
    .data = player_pixels,
    .size = {PLAYER_W, PLAYER_H},
    .palette = player_palette
};


/* ================== TILES ================== */
/* --- PLATFORM --- */
static uint8_t platform_pixels[TILE_SIZE * TILE_SIZE] = {
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,9,9,9,9,9,9,9,9,1,1,1,1,1,9,
    9,2,1,1,9,9,0,0,0,0,0,0,0,0,9,9,1,1,1,9,
    9,2,1,9,0,0,0,0,0,0,0,0,0,0,0,0,9,1,1,9,
    9,2,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,1,9,
    9,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,9,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static uint16_t platform_palette[] = {COL_NONE,
    0b1001001111001100,  // body
    0b0111101100001001,  // edge
    0,0,0,0,COL_WHITE, COL_BLACK
};

static PaletteImage TILE_PLATFORM_IMG = {
    .data = platform_pixels,
    .size = {TILE_SIZE, TILE_SIZE},
    .palette = platform_palette
};

/* --- GROUND --- */
static uint8_t ground_pixels[TILE_SIZE * TILE_SIZE] = {
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,9,
    9,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,9,
    9,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,9,
    9,4,3,3,3,3,3,3,3,3,2,3,3,3,3,3,3,3,3,9,
    9,4,3,2,2,2,3,3,2,2,1,2,3,3,2,2,2,3,3,9,
    9,2,2,1,1,1,2,2,1,1,1,1,2,2,1,1,1,2,2,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
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

/* --- BLOCK --- */
static uint8_t block_pixels[TILE_SIZE * TILE_SIZE] = {
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,9,
    9,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,9,
    9,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,9,
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
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
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,9,9,9,9,9,9,0,0,0,0,0,0,0,
    0,0,0,0,0,9,9,1,1,1,1,1,1,9,9,0,0,0,0,0,
    0,0,0,0,9,1,1,3,3,3,3,3,3,1,1,9,0,0,0,0,
    0,0,0,9,1,3,3,1,1,1,1,1,1,3,2,1,9,0,0,0,
    0,0,0,9,1,3,1,1,3,3,3,2,1,1,2,1,9,0,0,0,
    0,0,9,1,3,1,1,1,3,1,1,2,1,1,1,2,1,9,0,0,
    0,0,9,1,3,1,1,1,3,1,1,2,1,1,1,2,1,9,0,0,
    0,0,9,1,3,1,1,1,3,1,1,2,1,1,1,2,1,9,0,0,
    0,0,9,1,3,1,1,1,3,1,1,2,1,1,1,2,1,9,0,0,
    0,0,9,1,3,1,1,1,3,1,1,2,1,1,1,2,1,9,0,0,
    0,0,9,1,3,1,1,1,3,1,1,2,1,1,1,2,1,9,0,0,
    0,0,0,9,1,3,1,1,3,2,2,2,1,1,2,1,9,0,0,0,
    0,0,0,9,1,2,2,1,1,1,1,1,1,2,2,1,9,0,0,0,
    0,0,0,0,9,1,1,2,2,2,2,2,2,1,1,9,0,0,0,0,
    0,0,0,0,0,9,9,1,1,1,1,1,1,9,9,0,0,0,0,0,
    0,0,0,0,0,0,0,9,9,9,9,9,9,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
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

static PaletteImage *tile_images[] = {
    [TILE_EMPTY]    = NULL,
    [TILE_GROUND]   = &TILE_GROUND_IMG,
    [TILE_BLOCK]    = &TILE_BLOCK_IMG,
    [TILE_PLATFORM] = &TILE_PLATFORM_IMG,
    [TILE_BG_BLOCK] = &TILE_BG_BLOCK_IMG,
    [TILE_COIN]     = &TILE_COIN_IMG
};

/* ================= SECTIONS ================= */

static const uint16_t sections[][MAP_H][MAP_W] = {
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,5,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,3,0,0,2,0,0,0},
        {0,0,0,0,0,0,4,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
    }, {
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,5,0,5,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,5,0,0,3,2,2,2,3,0,0},
        {0,0,0,0,0,0,3,0,5,0,3,0,0},
        {0,0,0,2,0,0,3,0,2,0,3,0,0},
        {0,0,0,0,0,0,4,0,0,0,4,0,0},
        {0,0,0,0,0,0,4,0,0,0,4,0,0},
        {1,1,3,2,2,2,3,2,2,2,3,1,1},
        {1,1,3,0,0,0,3,0,0,0,3,1,1},
    }, {
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,5,0,0,0,0,5,0,0,0,0},
        {0,5,0,0,0,0,0,0,0,0,0,5,0},
        {0,0,0,3,2,2,2,2,2,3,0,0,0},
        {0,0,0,4,0,0,0,0,0,4,0,0,0},
        {0,2,0,4,0,5,5,5,0,4,0,2,0},
        {0,0,0,4,0,0,0,0,0,4,0,0,0},
        {1,1,1,3,2,0,2,0,2,3,1,1,1},
        {1,1,1,3,0,0,0,0,0,3,1,1,1},
    }, {
        {0,0,5,0,0,0,0,0,0,0,5,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,2,0,0,0,5,0,0,0,2,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,2,0,0,0,2,0,0,0,2,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
    }, {
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,5,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,3,0,3,0,0,0,0,0},
        {0,0,0,0,3,3,0,3,3,0,0,0,0},
        {0,0,0,3,3,3,5,3,3,3,0,0,0},
        {0,0,3,3,3,3,5,3,3,3,3,0,0},
        {1,1,1,1,1,1,5,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,1,1,1,1,1,1},
    }, {
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,5,0,3,2,2,2,2,2,2,3,0,0},
        {0,0,0,3,0,5,5,5,5,0,3,0,0},
        {0,2,2,3,2,3,3,3,3,3,3,0,0},
        {0,0,0,3,0,0,3,0,4,0,4,0,0},
        {0,0,0,3,0,0,4,0,3,0,4,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
    }/*, {
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
    }*/
};

#define SECTION_COUNT (sizeof(sections) / sizeof(sections[0]))

static int current_section;
static int section_col;
static float scroll_px;   // pixel-precise scroll offset

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

static void handle_death(struct ScrollerState *s) {
    snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);
    snprintf(high_buf, sizeof(high_buf), "H.Score: %d", s->highscore);
    snprintf(time_buf, sizeof(time_buf), "Time: %d", s->seconds_alive);
	if (s->score == 0) {
	    s->view = SCROLLER_VIEW_GAME_OVER;
	} else if (s->score > s->highscore) {
		s->highscore = s->score;
		s->view = SCROLLER_VIEW_NEW_SCORE;
	} else {
		s->view = SCROLLER_VIEW_GAME_OVER;
	}
}

static void handle_reset(struct ScrollerState *s) {
	s->player.pos = vec2(40, INFO_H + 100);
    s->player.vel = vec2(0, 0);
    s->player.on_ground = false;

	current_section = 0;
    section_col = 0;
    scroll_px = 0.0f;

    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            s->map[y][x] = sections[current_section][y][x];

	s->score = 0;
    s->seconds_alive = 0;
    s->scroll_timer = 0.0f;
    s->scroll_speed = 0.8f;

    snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);
    snprintf(high_buf, sizeof(high_buf), "H.Score: %d", s->highscore);
    snprintf(time_buf, sizeof(time_buf), "Time: %d", s->seconds_alive);
}

/* ================= Collision Helpers ================= */

// Block or ground → fully solid
bool solid_full(enum Tile t) {
    return t == TILE_BLOCK || t == TILE_GROUND;
}

// Only solid when landing from above
bool solid_top(enum Tile t) {
    return t == TILE_BLOCK || t == TILE_GROUND || t == TILE_PLATFORM;
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
            next_section = rand() % SECTION_COUNT;
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

    s->last_time_ms = to_ms_since_boot(get_absolute_time());;
    s->time_accum = 0.0f;

    font = mf_find_font("DejaVuSans12");
}

/* ================= STEP ================= */

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
    draw_string(screen, message, vec2(120, 50), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, score_buf, vec2(120, 62), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, high_buf, vec2(120, 74), COL_WHITE, font, MF_ALIGN_CENTER);
    draw_string(screen, "Again?", vec2(80, 100), COL_WHITE, font, MF_ALIGN_LEFT);
    draw_string(screen, "Quit", vec2(80, 114), COL_WHITE, font, MF_ALIGN_LEFT);
    draw_string(screen, ">", vec2(70, 100 + 14 * s->selected), COL_WHITE, font, MF_ALIGN_LEFT);
    if (s->selected == 0 && go) {
        handle_reset(s);
        s->view = SCROLLER_VIEW_PLAYING;
    }
    if (s->selected == 1 && go) return false;
    return true;
}

bool Scroller_main(struct ScrollerState *s, Screen screen) {
    /* -------- TIME -------- */
    static long frame_start = 0;

    long now = to_ms_since_boot(get_absolute_time());
    if (frame_start == 0)
        frame_start = now;

    float dt = (now - frame_start) / 1000.0f;
    frame_start = now;
    s->time_accum += dt;

    /* -------- SCROLL -------- */
    scroll_px += dt * s->scroll_speed * TILE_SIZE;

    while (scroll_px >= TILE_SIZE) {
        scroll_px -= TILE_SIZE;
        shift_map_left(s);
    }

    while (s->time_accum >= 1.0f) {
        s->time_accum -= 1.0f;
        s->seconds_alive++;
        s->score += 5;

        if (s->seconds_alive % 60 == 0)
            s->score += 50;

        s->scroll_speed += 0.01f;

        snprintf(time_buf, sizeof(time_buf), "Time: %d", s->seconds_alive);
        snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);
    }

    /* -------- INPUT -------- */
    s->player.vel.x = 0;
    if (keypad_get(1,1).held) s->player.vel.x = -MOVE_SPEED;
    if (keypad_get(3,1).held) s->player.vel.x = MOVE_SPEED;
    if (keypad_get(2,0).pressed && s->player.on_ground) {
        s->player.vel.y = JUMP_VEL;
        s->player.on_ground = false;
    }
    keypad_next_frame();

    /* -------- PHYSICS -------- */
    s->player.vel.y += GRAVITY;
    Vec2 next = s->player.pos;

    /* --- X AXIS (side collisions) --- */
    next.x += s->player.vel.x;

    if (s->player.vel.x >= 0) { // moving right
        if (solid_side(get_tile(s, next.x + PLAYER_W - 1, next.y)) ||
            solid_side(get_tile(s, next.x + PLAYER_W - 1, next.y + PLAYER_H - 1))) {
            next.x = ((next.x + PLAYER_W - 1 + (int)scroll_px) / TILE_SIZE) * TILE_SIZE
                     - PLAYER_W - (int)scroll_px;
        }
    } else if (s->player.vel.x < 0) { // moving left
        if (solid_side(get_tile(s, next.x, next.y)) ||
            solid_side(get_tile(s, next.x, next.y + PLAYER_H - 1))) {
            next.x = ((next.x + (int)scroll_px) / TILE_SIZE + 1) * TILE_SIZE
                     - (int)scroll_px;
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
    if ((solid_top(b1) || solid_top(b2)) && s->player.vel.y > 0) {
        // Align bottom of player to top of tile
        int tile_y = ((next.y + PLAYER_H) / TILE_SIZE) * TILE_SIZE;
        next.y = tile_y - PLAYER_H;

        s->player.vel.y = 0;
        s->player.on_ground = true;
    }
    else {
        s->player.on_ground = false;
    }

    // Ceiling collision (jumping into blocks)
    enum Tile t1 = get_tile(s, next.x + 2, next.y);
    enum Tile t2 = get_tile(s, next.x + PLAYER_W - 2, next.y);
    if (((solid_full(t1) || solid_full(t2)) && s->player.vel.y < 0) || next.y < INFO_H + 1) {
        next.y = ((next.y) / TILE_SIZE + 1) * TILE_SIZE;
        s->player.vel.y = 0;
    }


    /* --- COINS --- */
    if (get_tile(s, next.x + PLAYER_W / 2, next.y + PLAYER_H / 2) == TILE_COIN) {
        set_tile(s, next.x + PLAYER_W / 2, next.y + PLAYER_H / 2, TILE_EMPTY);
        s->score += 100;
        snprintf(score_buf, sizeof(score_buf), "Score: %d", s->score);
    }

    s->player.pos = next;

    /* -------- VERTICAL DEATH -------- */
    if (s->player.pos.y > SCREEN_HEIGHT)
		handle_death(s);

    /* -------- DRAW -------- */
    draw_rect(screen, vec2(0, INFO_H), vec2(SCREEN_W, SCREEN_H - INFO_H), COL_BG);

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            PaletteImage *img = tile_images[s->map[y][x]];
            if (!img) continue;
            draw_palette(
                screen, *img,
                vec2(
                    x * TILE_SIZE - (int)scroll_px,
                    INFO_H + y * TILE_SIZE
                )
            );
        }
    }

	draw_yline(screen, vec2(0, INFO_H), SCREEN_H - INFO_H, COL_RED);
	draw_yline(screen, vec2(0, 0), INFO_H, COL_WHITE);
	draw_yline(screen, vec2(SCREEN_W - 1, 0), INFO_H, COL_WHITE);
    draw_xline(screen, vec2(0, INFO_H), SCREEN_W, COL_WHITE);
    draw_xline(screen, vec2(0, 0), SCREEN_W, COL_WHITE);

    draw_palette(
        screen, PLAYER_IMG,
        s->player.pos
    );
    draw_string(screen, score_buf, vec2(2,3), COL_WHITE, font, MF_ALIGN_LEFT);
    draw_string(screen, high_buf, vec2(-2,3), COL_WHITE, font, MF_ALIGN_RIGHT);
    draw_string(screen, time_buf, vec2(SCREEN_W / 2,3), COL_WHITE, font, MF_ALIGN_CENTER);

    long frame_end = to_ms_since_boot(get_absolute_time());
    long frame_time = frame_end - now;

    if (frame_time < FRAME_TIME_MS) {
        sleep_ms(FRAME_TIME_MS - frame_time);
    }

    return true;
}

bool Scroller_step(struct ScrollerState *s, Screen screen) {
	switch (s->view) {
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
