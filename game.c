#include "game.h"
#include <stdint.h>
#include <stdio.h>

#define CORNFLOWER_BLUE 0x6495ed
#define LIGHT_SALMON 0xffa07a
#define YELLOW_GREEN 0x9acd32

const uint32_t speed = 200;
int32_t x = 0, y = 0;
uint8_t w = 20;

typedef struct Rect {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} Rect;

uint32_t player_sprite[100] = {
    0xFF,     0x800000FF, 0x800000FF, 0xFF,     0xFFFFFFFF, 0xFFFFFFFF,
    0xFF,     0xFF,       0xFF,       0xFF,     0,          0,
    0,        0,          0,          0,        0,          0,
    0,        0,          0,          0,        0xffa07a,   0xffa07a,
    0xffa07a, 0xffa07a,   0xffa07a,   0xffa07a, 0,          0,
    0,        0,          0xffa07a,   0xffa07a, 0xffa07a,   0xffa07a,
    0xffa07a, 0xffa07a,   0,          0,        0,          0,
    0xffa07a, 0xffa07a,   0xffa07a,   0xffa07a, 0xffa07a,   0xffa07a,
    0,        0,          0,          0,        0xffa07a,   0xffa07a,
    0xffa07a, 0xffa07a,   0xffa07a,   0xffa07a, 0,          0,
    0,        0,          0xffa07a,   0xffa07a, 0xffa07a,   0xffa07a,
    0xffa07a, 0xffa07a,   0,          0,        0,          0,
    0xffa07a, 0xffa07a,   0xffa07a,   0xffa07a, 0xffa07a,   0xffa07a,
    0,        0,          0,          0,        0,          0,
    0,        0,          0,          0,        0,          0,
    0,        0,          0,          0,        0,          0,
    0,        0,          0,          0,
};

struct Rect player_rect = {20, 20, 10, 10};

void fill_color(Buffer *buffer, uint32_t color) {
    for (uint32_t i = 0; i < buffer->w * buffer->h; i++) {
        buffer->data[i] = color;
    }
}

void draw_rect(Buffer *buffer, uint32_t color, uint16_t w, uint16_t h,
               uint16_t x, uint16_t y) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            buffer->data[(y + j) * buffer->w + (x + i)] = color;
        }
    }
}

void draw_spr(Buffer *buffer, uint32_t *data, Rect rect) {
    uint32_t idx = 0;
    for (int j = 0; j < rect.h; j++) {
        for (int i = 0; i < rect.w; i++) {
            if (data[idx] == 0xFFFF00FF) {
                idx++;
                continue;
            }

            buffer->data[(y + j) * buffer->w + (x + i)] = data[idx++];
        }
    }
}

void game_update_and_render(Buffer *buffer, Input *input, float elapsed) {

    if (input->down.is_pressed) {
        y += speed * elapsed;
        if ((uint32_t)y > buffer->h - w) {
            y = buffer->h - w;
        }
    }

    if (input->up.is_pressed) {
        y -= speed * elapsed;
        if (y < 0) {
            y = 0;
        }
    }

    if (input->left.is_pressed) {
        x -= speed * elapsed;
        if (x < 0) {
            x = 0;
        }
    }

    if (input->right.is_pressed) {
        x += speed * elapsed;
        if ((uint32_t)x > buffer->w - w) {
            x = buffer->w - w;
        }
    }

    fill_color(buffer, CORNFLOWER_BLUE);
    // draw_rect(buffer, LIGHT_SALMON, w, w, x, y);
    draw_spr(buffer, player_sprite, player_rect);
}

void game_init(void) {}
