#ifndef _DRAWING_H
#define _DRAWING_H
#include <stdint.h>
#include <stdbool.h>
#include "../lib/mcufont/mf_justify.h"

typedef struct {
    uint16_t *buffer;
} Screen;

typedef struct {
    int x, y;
} Vec2;

typedef struct {
    uint8_t *data;
    Vec2 size;
    uint16_t *palette;
} PaletteImage;

typedef struct {
    bool *data;
    Vec2 size;
} MaskImage;


Vec2 vec2(int x, int y);
void draw_xline(Screen s, Vec2 pos, int len, uint16_t col);
void draw_yline(Screen s, Vec2 pos, int len, uint16_t col);
void draw_line(Screen s, Vec2 a, Vec2 b, uint16_t col);
void draw_rect(Screen s, Vec2 pos, Vec2 size, uint16_t colour);
void draw_mask(Screen s, MaskImage img, Vec2 pos, uint16_t colour);
void draw_palette(Screen s, PaletteImage img, Vec2 pos);
void draw_string(Screen s, const char *str, Vec2 pos, uint16_t colour, const struct mf_font_s *font, enum mf_align_t align);
void draw_string_multiline(Screen s, const char *str, Vec2 pos, uint16_t colour, const struct mf_font_s *font);
inline Vec2 vec2_add(Vec2 a, Vec2 b) {
    return vec2(a.x + b.x, a.y + b.y);
}
#endif