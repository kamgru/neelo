#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Buffer {
    uint32_t w;
    uint32_t h;
    uint32_t *data;
} Buffer;


typedef struct BtnInfo {
    bool is_pressed;
    bool was_pressed;
} BtnInfo;

typedef struct {
    struct BtnInfo down;
    struct BtnInfo up;
    struct BtnInfo left;
    struct BtnInfo right;
} Input;

void game_update_and_render(Buffer *buffer, Input *input, float elapsed);

#endif
