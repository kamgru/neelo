#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SCREEN_W 1024
#define SCREEN_H 768
#define CORNFLOWER_BLUE 0x6495ed
#define LIGHT_SALMON 0xffa07a
#define YELLOW_GREEN 0x9acd32
#define FPS 60

void fill_color(uint32_t *image, uint32_t color) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        image[i] = color;
    }
}

void draw_rect(uint32_t *buffer, uint32_t color, uint16_t w, uint16_t h,
               uint16_t x, uint16_t y) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            buffer[(y + j) * SCREEN_W + (x + i)] = color;
        }
    }
}

typedef struct BtnInfo {
    bool is_pressed;
    bool was_pressed_prev_frame;
} BtnInfo;

typedef struct {
    struct BtnInfo down;
    struct BtnInfo up;
    struct BtnInfo left;
    struct BtnInfo right;
} Input;

// returns milliseconds since epoch
uint64_t get_time(void) {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

const uint32_t speed = 200;

int main(void) {
    Display *display;
    Window window;
    XEvent evt;
    GC gc;
    int screen_num;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    uint32_t *image = malloc(SCREEN_W * SCREEN_H * sizeof(uint32_t));
    fill_color(image, YELLOW_GREEN);
    draw_rect(image, LIGHT_SALMON, 20, 20, 10, 10);

    screen_num = DefaultScreen(display);

    unsigned long border_color = BlackPixel(display, screen_num);
    unsigned long background_color = WhitePixel(display, screen_num);

    window = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                                 10, // initial x position
                                 10, // initial y position
                                 SCREEN_W, SCREEN_H,
                                 1, // border width
                                 border_color, background_color);

    gc = XCreateGC(
        display, window,
        0,     // valuemask - specifies addtional components to be set
        NULL); // values - if valuemask is set, then there we pass the data

    XImage *ximg = XCreateImage(display, DefaultVisual(display, screen_num),
                                DefaultDepth(display, screen_num), ZPixmap, 0,
                                (char *)image, SCREEN_W, SCREEN_H, 32, 0);

    XStoreName(display, window, "My X11 Window!");

    XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask);

    XMapWindow(display, window);

    const float frame_duration = 1000.0f / FPS;

    Input input = {0};
    int32_t x = 0, y = 0;
    uint8_t w = 20;

    uint64_t last_time = get_time();

    bool should_quit = false;
    while (!should_quit) {

        uint64_t current_time = get_time();
        float elapsed = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        while (XPending(display) > 0) {
            XNextEvent(display, &evt);

            if (evt.type == KeyPress) {
                KeySym key = XLookupKeysym(&evt.xkey, 0);

                if (key == XK_Escape) {
                    should_quit = 1;
                }

                if (key == XK_s) {
                    if (input.down.was_pressed_prev_frame == false) {
                        input.down.is_pressed = true;
                        input.down.was_pressed_prev_frame = true;
                    }
                }

                if (key == XK_w) {
                    if (input.up.was_pressed_prev_frame == false) {
                        input.up.is_pressed = true;
                        input.up.was_pressed_prev_frame = true;
                    }
                }

                if (key == XK_a) {
                    if (input.left.was_pressed_prev_frame == false) {
                        input.left.is_pressed = true;
                        input.left.was_pressed_prev_frame = true;
                    }
                }

                if (key == XK_d) {
                    if (input.right.was_pressed_prev_frame == false) {
                        input.right.is_pressed = true;
                        input.right.was_pressed_prev_frame = true;
                    }
                }

            } else if (evt.type == KeyRelease) {
                KeySym key = XLookupKeysym(&evt.xkey, 0);

                if (key == XK_s) {
                    input.down.is_pressed = false;
                }
                if (key == XK_w) {
                    input.up.is_pressed = false;
                }
                if (key == XK_a) {
                    input.left.is_pressed = false;
                }
                if (key == XK_d) {
                    input.right.is_pressed = false;
                }
            }
        }

        // game update and render

        if (input.down.is_pressed) {
            y += speed * elapsed;
            if (y > SCREEN_H - w) {
                y = SCREEN_H - w;
            }
        }
        if (input.up.is_pressed) {
            y -= speed * elapsed;
            if (y < 0) {
                y = 0;
            }
        }
        if (input.left.is_pressed) {
            x -= speed * elapsed;
            if (x < 0) {
                x = 0;
            }
        }
        if (input.right.is_pressed) {
            x += speed * elapsed;
            if (x > SCREEN_W - w) {
                x = SCREEN_W - w;
            }
        }

        fill_color(image, CORNFLOWER_BLUE);
        draw_rect(image, LIGHT_SALMON, w, w, x, y);

        // end game update and render

        XPutImage(display, window, gc, ximg, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

        input.up.was_pressed_prev_frame = input.up.is_pressed;
        input.down.was_pressed_prev_frame = input.down.is_pressed;
        input.left.was_pressed_prev_frame = input.left.is_pressed;
        input.right.was_pressed_prev_frame = input.right.is_pressed;

        uint64_t end_time = get_time();

        elapsed = (end_time - current_time) / 1000.0f;
        float sleep_time = frame_duration - elapsed * 1000;

        if (sleep_time > 0) {
            usleep(sleep_time * 1000);
        }
    }

    free(image);
    ximg->data = NULL;
    XDestroyImage(ximg);
    XCloseDisplay(display);
    return 0;
}
