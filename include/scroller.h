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

enum Tile {
    TILE_EMPTY = 0,
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
    Vec2 pos;
    Vec2 vel;
    bool on_ground;
    int lives;
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
