#include <stdbool.h>
#include "../include/drawing.h"
#include "../include/keypad.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/time.h"

struct Cell {
    uint16_t value;
    bool merged;
    Vec2 moved_from;
};

struct Anim {
    Vec2 from, to;
};

struct GameState {
    struct Cell board[4*4];
    uint16_t anim_frame;
};

const uint32_t ANIM_FRAMES = 3;

void add_random_tile(struct GameState *state) {
    bool space = false;
    for (uint32_t i = 0; i < 4*4; i++) {
        if (state->board[i].value == 0) {
            space = true;
            break;
        }
    }
    if (!space) {
        return;
    }
    while (1) {
        uint32_t rand_pos = rand() % (4*4);
        if (state->board[rand_pos].value == 0) {
            state->board[rand_pos].value = rand() % 10 == 0 ? 4 : 2; // 10% chance of a 4
            break;
        }
    }
}

void two048_init(struct GameState *state) {
    srand(to_us_since_boot(get_absolute_time()));
    state->anim_frame = 0;
    memset(state->board, 0, 4*4*sizeof(*state->board));
    add_random_tile(state);
    add_random_tile(state);
}

void move(struct GameState *state, Vec2 dir) {
    uint32_t tiles_moved = 0;
    for (uint32_t x = 0; x < 4; x++) {
        for (uint32_t y = 0; y < 4; y++) {
            state->board[y * 4 + x].moved_from = vec2(-1, -1);
        }
    }
    for (uint32_t i = 0; i < 3; i++) {
        for (int32_t a = 0; a < 4; a++) {
            for (int32_t b = 0; b < 3; b++) {
                uint32_t this, prev;
                Vec2 this_v, prev_v;
                if (dir.x == 1) {
                    this_v = vec2(3 - b, a);
                    prev_v = vec2(2 - b, a);
                } else if (dir.x == -1) {
                    this_v = vec2(b, a);
                    prev_v = vec2(b + 1, a);
                } else if (dir.y == 1) {
                    this_v = vec2(a, 3 - b);
                    prev_v = vec2(a, 2 - b);
                } else if (dir.y == -1) {
                    this_v = vec2(a, b);
                    prev_v = vec2(a, b + 1);
                }
                this = this_v.y * 4 + this_v.x;
                prev = prev_v.y * 4 + prev_v.x;
                if (state->board[this].value == 0 && state->board[prev].value != 0) {
                    state->board[this] = state->board[prev];
                    if (state->board[this].moved_from.x < 0) {
                        state->board[this].moved_from = prev_v;
                    }
                    state->board[prev] = (struct Cell){.value = 0, .merged = false, .moved_from = vec2(-1, -1)};
                    tiles_moved++;
                } else if (state->board[this].value != 0 && state->board[this].value == state->board[prev].value && !state->board[this].merged && !state->board[prev].merged) {
                    state->board[this].value *= 2;
                    state->board[this].merged = true;
                    state->board[this].moved_from = prev_v;
                    state->board[prev] = (struct Cell){.value = 0, .merged = false, .moved_from = vec2(-1, -1)};
                    tiles_moved++;
                }
            }
        }
    }
    if (tiles_moved > 0) {
        state->anim_frame = ANIM_FRAMES;
    }
}


uint32_t colour_index(uint16_t number) {
    uint32_t count = 0;
    number >>= 1;
    number >>= 1;
    while (number > 0) {
        count++;
        number >>= 1;
    }
    return count;
}

uint16_t bg_colours[] = {
    0xef3b, 0xef16, 0xf58f, 0xf4ac, 0xf3eb, 0xf2e7, 0xee6e, 0xee6c, 0xee4a, 0xee27, 0xee05, 0x39c6
};

uint16_t fg_colours[] = {
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
};

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

Vec2 vec2_lerp(Vec2 a, Vec2 b, float t) {
    return vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

bool two048_step(struct GameState *state, Screen s) {
    if (state->anim_frame == 0) {
        if (keypad_get(2, 0).pressed) {
            move(state, vec2(0, -1));
        } else if (keypad_get(2, 2).pressed) {
            move(state, vec2(0, 1));
        } else if (keypad_get(1, 1).pressed) {
            move(state, vec2(-1, 0));
        } else if (keypad_get(3, 1).pressed) {
            move(state, vec2(1, 0));
        }
    }
    keypad_next_frame();
    
    static const struct mf_font_s *font = NULL; // TODO not static....
    if (font == NULL) font = mf_find_font("DejaVuSans12");

    for (uint32_t x = 0; x < 4; x++) {
        for (uint32_t y = 0; y < 4; y++) {
            if (state->board[y * 4 + x].value != 0) {
                char st[7];
                sprintf(st, "%d", state->board[y * 4 + x].value);
                uint16_t fg_colour = fg_colours[colour_index(state->board[y * 4 + x].value)];
                uint16_t bg_colour = bg_colours[colour_index(state->board[y * 4 + x].value)];
                Vec2 origin_pos;
                Vec2 moved_from = state->board[y * 4 + x].moved_from;
                Vec2 to_pos = vec2(x * 60 + 5, y * 60 + 5);
                if (moved_from.x != -1) {
                    Vec2 from_pos = vec2_add(vec2_mul(moved_from, 60), vec2(5, 5));
                    origin_pos = vec2_lerp(to_pos, from_pos, (float)state->anim_frame / (float)ANIM_FRAMES);
                } else {
                    origin_pos = to_pos;
                }
                draw_rect(s, origin_pos, vec2(50, 50), bg_colour);
                draw_string(s, st, vec2_add(origin_pos, vec2(25, 25 - 6)), fg_colour, font, MF_ALIGN_CENTER);
            }
            state->board[y * 4 + x].merged = false;
        }
    }
    if (state->anim_frame > 0) {
        state->anim_frame--;
        if (state->anim_frame == 0) {
            add_random_tile(state);
        }
    }
    
    return true;
}
