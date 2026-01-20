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
