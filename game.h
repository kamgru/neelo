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

typedef struct {
	uint32_t w;
	uint32_t h;
} ScreenResolution;

void game_init(void);
void game_update_and_render(Buffer *buffer, Input *input, float elapsed);

void platform_test(char* arg);
uint32_t *platform_read_bmp(char *filename);
ScreenResolution* platform_get_screen_resolutions(uint32_t* num);
void platform_set_screen_resolution(ScreenResolution res, bool fullscreen);

#endif
