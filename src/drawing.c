// #include "../fonts/fonts.h"
#include "../lib/mcufont/mcufont.h"
#include "../include/drawing.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

typedef struct {
    uint16_t *buffer;
    uint16_t colour;
    const struct mf_font_s *font;
    uint16_t x, y;
} FontData;

static bool invalid_pixel(int16_t i) {
    return (i < 0 || i >= 240);
}

static inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}


static inline uint8_t lerp8(uint8_t a, uint8_t b, uint16_t t, uint16_t max) {
    return a + ((b - a) * t) / max;
}

static inline uint16_t lerp565(uint16_t c1, uint16_t c2, int t, int max) {
    int r1 = (c1 >> 11) & 31, g1 = (c1 >> 5) & 63, b1 = c1 & 31;
    int r2 = (c2 >> 11) & 31, g2 = (c2 >> 5) & 63, b2 = c2 & 31;

    int r = r1 + (r2 - r1) * t / max;
    int g = g1 + (g2 - g1) * t / max;
    int b = b1 + (b2 - b1) * t / max;

    return (r << 11) | (g << 5) | b;
}


static inline uint16_t pack565(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 11) | (g << 5) | b;
}

static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state) {
    FontData *data = state;
    uint8_t r = ((data->colour >> 11) & 0b11111) * alpha / 255; // TODO a >> 8 may be acceptable here
    uint8_t g = ((data->colour >> 5) & 0b111111) * alpha / 255;
    uint8_t b = ((data->colour >> 0) & 0b11111) * alpha / 255;
    while (count--) {
        uint16_t bg = data->buffer[y * 240 + x];
        uint8_t bg_r = ((bg >> 11) & 0b11111) * (255 - alpha) / 255 + r;
        uint8_t bg_g = ((bg >> 5) & 0b111111) * (255 - alpha) / 255 + g;
        uint8_t bg_b = ((bg >> 0) & 0b11111) * (255 - alpha) / 255 + b;

        data->buffer[y * 240 + x] = (bg_r << 11) | (bg_g << 5) | bg_b;
        x++;
    }
}

static uint8_t char_callback(int16_t x0, int16_t y0, mf_char character, void *state) {
    FontData *data = state;
    return mf_render_character(data->font, x0, y0, character, &pixel_callback, state);
}

void draw_string(Screen s, const char *str, Vec2 pos, uint16_t colour, const struct mf_font_s *font, enum mf_align_t align) {
    FontData state = (FontData){
        .buffer = s.buffer,
        .colour = colour,
        .font = font,
    };
    mf_render_aligned(
        font,
        pos.x, pos.y,
        align,
        str, strlen(str),
        &char_callback, &state);
}

bool _multiline_string_callback(const char *str, uint16_t char_count,
                                void *state) {
    FontData *data = state;
    mf_render_aligned(data->font, data->x, data->y, MF_ALIGN_LEFT, str,
                      char_count, &char_callback, state);
    data->y += 12;
    return true;
}

void draw_string_multiline(Screen s, const char *str, Vec2 pos, uint16_t colour, const struct mf_font_s *font) {
    FontData state = (FontData){
        .buffer = s.buffer,
        .colour = colour,
        .font = font,
        .x = pos.x,
        .y = pos.y,
    };
    mf_wordwrap(font, 240, str, &_multiline_string_callback, &state);
}

uint16_t get_px(Screen s, Vec2 pos) {
    return s.buffer[pos.y * 240 + pos.x];
}

void draw_px(Screen s, Vec2 pos, uint16_t col) {
    s.buffer[pos.y * 240 + pos.x] = col;
}

void draw_xline(Screen s, Vec2 pos, int len, uint16_t col) {
    if (invalid_pixel(pos.y)) return;
    for (int x = pos.x; x < pos.x + len; x++) {
        if (invalid_pixel(x)) continue;
        s.buffer[pos.y * 240 + x] = col;
    }
}

void draw_yline(Screen s, Vec2 pos, int len, uint16_t row) {
    if (invalid_pixel(pos.x)) return;
    for (int y = pos.y; y < pos.y + len; y++) {
        if (invalid_pixel(y)) continue;
        s.buffer[y * 240 + pos.x] = row;
    }
}

void draw_line(Screen s, Vec2 a, Vec2 b, uint16_t colour) {
    int dx =  abs(b.x-a.x);
    int sx = a.x<b.x ? 1 : -1;
    int dy = -abs(b.y-a.y);
    int sy = a.y<b.y ? 1 : -1;
    int err = dx+dy;
    int e2;

    while (true) {
        if (invalid_pixel(a.x) || invalid_pixel(a.y)) continue;
        s.buffer[a.y * 240 + a.x] = colour;
        if (a.x == b.x && a.y == b.y)
            break;
        e2 = 2*err;

        if (e2 >= dy) {
            err += dy;
            a.x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            a.y += sy;
        }
    }
}

Vec2 vec2(int x, int y) {
    return (Vec2){x, y};
}
Vec3 vec3(int x, int y, int z) {
    return (Vec3){x, y, z};
}

void draw_rect(Screen s, Vec2 pos, Vec2 size, uint16_t colour) {
    for (int y = pos.y; y < pos.y + size.y; y++) {
        for (int x = pos.x; x < pos.x + size.x; x++) {
            if (invalid_pixel(x) || invalid_pixel(y)) continue;
            s.buffer[y * 240 + x] = colour;
        }
    }
}

void draw_circle(Screen s, Vec2 pos, uint16_t radius, uint16_t colour) {
    if (radius < 0) radius = -radius;
    int r2 = radius * radius;

    int y_min = pos.y - radius;
    int y_max = pos.y + radius;

    if (y_min < 0) y_min = 0;
    if (y_max >= SCREEN_HEIGHT) y_max = SCREEN_HEIGHT - 1;

    for (int y = y_min; y <= y_max; y++) {
        int dy = y - pos.y;
        int dx_max = (int)sqrtf(r2 - dy * dy);

        int x1 = pos.x - dx_max;
        int x2 = pos.x + dx_max;

        if (x1 < 0) x1 = 0;
        if (x2 >= SCREEN_WIDTH) x2 = SCREEN_WIDTH - 1;

        uint16_t *row = &s.buffer[y * SCREEN_WIDTH];
        for (int x = x1; x <= x2; x++) {
            row[x] = colour;
        }
    }
}


void draw_mask(Screen s, MaskImage img, Vec2 pos, const uint16_t *colours) {
    for (int i = 0; i < img.size.y; i++) {
        for (int j = 0; j < img.size.x; j++) {
            uint8_t pix = img.data[i * img.size.x + j];
            if (invalid_pixel(pos.x+j) || invalid_pixel(pos.y+i)) continue;
            if (pix != 0) {
                s.buffer[pos.x + j + (pos.y + i) * 240] = colours[pix - 1];
            }
        }
    }
}
// void draw_transparent_packed(uint16_t *data, int width, int x, int y, int colour, uint16_t *buffer) {
//     for (int i = 0; i < width; i++) {
//         uint16_t line = data[i];
//         for (int j = 0; j < width; j++) {
//     if (line & 1) {
//         buffer[x + j + (y + i) * 240] = colour;
//     }
//             line >>= 1;
//     }
//     }
// }

void draw_palette(Screen s, PaletteImage img, Vec2 pos) {
    for (int i = 0; i < img.size.y; i++) {
        for (int j = 0; j < img.size.x; j++) {
            if (invalid_pixel(pos.x+j) || invalid_pixel(pos.y+i)) continue;
            //     printf("Out of range (coloured) %d %d %d %d\n", x, j, y, i);
            // }
            unsigned char px = img.data[i * img.size.x + j];
            int colour = img.palette[px];
            if (colour != 0b0000100000100001) s.buffer[pos.x + j + (pos.y + i) * 240] = colour;
        }
    }
}

static const uint8_t dither4x4[4][4] = {
    { 0,  8,  2, 10 },
    {12,  4, 14,  6 },
    { 3, 11,  1,  9 },
    {15,  7, 13,  5 }
};


void draw_gradient(
    Screen s,
    Vec2 pos,
    Vec2 size,
    uint16_t col1,
    uint16_t col2,
    Direction dir
) {
    int w = size.x;
    int h = size.y;
    int max = w + h;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {

            int t;
            switch (dir) {
            case UP:         t = h - y; break;
            case DOWN:       t = y; break;
            case LEFT:       t = w - x; break;
            case RIGHT:      t = x; break;
            case UP_LEFT:    t = (w - x) + (h - y); break;
            case UP_RIGHT:   t = x + (h - y); break;
            case DOWN_LEFT:  t = (w - x) + y; break;
            case DOWN_RIGHT: t = x + y; break;
            default: t = 0;
            }

            // smooth interpolation
            uint16_t base = lerp565(col1, col2, t, max);

            // subtle ordered dither (565 precision)
            int d = dither4x4[y & 3][x & 3];
            if (d > 8 && base != col2)
                base++;

            int px = pos.x + x;
            int py = pos.y + y;
            if (!invalid_pixel(px) && !invalid_pixel(py))
                s.buffer[py * SCREEN_WIDTH + px] = base;
        }
    }
}

// TODO these would probably benefit from inlining
// see https://stackoverflow.com/questions/60133817/how-do-i-inline-a-function-from-another-translation-unit
Vec2 vec2_add(Vec2 a, Vec2 b) {
    return vec2(a.x + b.x, a.y + b.y);
}

bool vec2_eq(Vec2 a, Vec2 b) {
    return a.x == b.x && a.y == b.y;
}

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

bool vec3_eq(Vec3 a, Vec3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

Vec2f vec2f_add(Vec2f a, Vec2f b) {
    return vec2f(a.x + b.x, a.y + b.y);
}

bool vec2f_eq(Vec2f a, Vec2f b) {
    return a.x == b.x && a.y == b.y;
}

Vec2 vec2f_to_vec2(struct Vec2f v) {
    return vec2((int)v.x, (int)v.y);
}

Vec2f vec2f(float x, float y) {
    return (Vec2f){.x = x, .y = y};
}
