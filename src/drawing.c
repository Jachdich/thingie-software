// #include "../fonts/fonts.h"
#include "../lib/mcufont/mcufont.h"
#include "../include/drawing.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint16_t *buffer;
    uint16_t colour;
    const struct mf_font_s *font;
    uint16_t x, y;
} FontData;

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

void draw_xline(Screen s, Vec2 pos, int len, uint16_t col) {
    for (int x = pos.x; x < pos.x + len; x++) {
        s.buffer[pos.y * 240 + x] = col;
    }
}

void draw_yline(Screen s, Vec2 pos, int len, uint16_t row) {
    for (int y = pos.y; y < pos.y + len; y++) {
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

void draw_rect(Screen s, Vec2 pos, Vec2 size, uint16_t colour) {
    for (int y = pos.y; y < pos.y + size.y; y++) {
        for (int x = pos.x; x < pos.x + size.x; x++) {
            s.buffer[y * 240 + x] = colour;
        }
    }
}

void draw_mask(Screen s, MaskImage img, Vec2 pos, const uint16_t *colours) {
    for (int i = 0; i < img.size.y; i++) {
        for (int j = 0; j < img.size.x; j++) {
            uint8_t pix = img.data[i * img.size.x + j];
            if (pos.x + j >= 240 || pos.y + i >= 240) continue;
            if (pos.x + j < 0 || pos.y + i < 0) continue;
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
            if (pos.x + j >= 240 || pos.y + i >= 240) continue;
            if (pos.x + j < 0 || pos.y + i < 0) continue;
            //     printf("Out of range (coloured) %d %d %d %d\n", x, j, y, i);
            // }
            unsigned char px = img.data[i * img.size.x + j];
            s.buffer[pos.x + j + (pos.y + i) * 240] = img.palette[px];
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
