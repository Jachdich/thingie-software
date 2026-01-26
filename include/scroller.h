//
// Created by ignacy on 12/01/2026.
//

#ifndef _SCROLLER_H
#define _SCROLLER_H

#include <stdbool.h>
#include <stdint.h>
#include "../include/drawing.h"

#define TILE_SIZE 20
#define SCREEN_W 240
#define SCREEN_H 240
#define INFO_H 20

#define MAP_W 13
#define MAP_H 11

#define MAX_STARS 35
#define MAX_DECORS 30
#define MIN_Z 0
#define MAX_Z 10

enum Tile {
    TILE_EMPTY,
    TILE_GROUND,
    TILE_DIRT,
    TILE_PLATFORM,
    TILE_BLOCK,
    TILE_BG_BLOCK,
    TILE_BREAKABLE,
    TILE_COIN,
    TILE_HEART,
    TILE_SPIKE
};

enum Decor {
    DECOR_GRASS,
    DECOR_CLOUD,
    DECOR_COUNT
};

typedef struct {
    PaletteImage *data[8];
    uint8_t size;
    uint8_t chance;
    uint8_t z;
} DecorImage;

typedef struct {
    Vec2f pos;           // precise position on screen/world
    uint8_t type;        // e.g., DECOR_GRASS
    uint8_t image_idx;   // which image from decor_images[type]
    uint8_t z;           // z-index for layering
} DecorInstance;


typedef struct {
    int16_t x, y;
    uint8_t size;        // 0–2
    uint8_t speed;       // base speed
    uint8_t frac;        // 0–9 (tenths of a pixel)
    uint8_t layer;       // parallax layer
    float flicker_timer;
    bool off;
} Star;


enum SkyType {
    SKY_NIGHT,
    SKY_MIDNIGHT,
    SKY_DAWN,
	SKY_GRADIENT,
    SKY_COUNT
};

struct Player {
    Vec2f pos;
    Vec2f vel;
    bool on_ground;
    int lives;
    float move_speed;
	int invulnerable;
};

typedef enum {
    SCROLLER_VIEW_PAUSED,
    SCROLLER_VIEW_GAME_OVER,
    SCROLLER_VIEW_NEW_SCORE,
    SCROLLER_VIEW_PLAYING,
    SCROLLER_VIEW_INIT
} Scroller_View;

struct ScrollerState {
	int selected;
	Scroller_View view;

    enum SkyType sky;
    Star stars[MAX_STARS];
    DecorInstance decors[MAX_DECORS];
    uint8_t decor_count;

    struct Player player;
    uint16_t map[MAP_H][MAP_W];

    int score;
    int highscore;
    int seconds_alive;

    long frame_start;
    int last_time_ms;
    float time_accum;

    float scroll_timer;
    float scroll_speed; // tiles per second
};


void Scroller_init(struct ScrollerState *state);
bool Scroller_step(struct ScrollerState *state, Screen s);

#endif
