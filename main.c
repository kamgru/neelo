#include "game.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SCREEN_W 2560
#define SCREEN_H 1440
#define FPS 60

void platform_test(char *arg) { printf("[platform]: %s\n", arg); }

// supports only 24bpp bmps with no compression and size power of 2
// any other will crash the game ;)
uint32_t *platform_read_bmp(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("failed to open file %s\n", filename);
        fclose(file);
        return NULL;
    }

    uint8_t bmp_file_header[14];
    fread(bmp_file_header, 14, 1, file);
    if (bmp_file_header[0] != 0x42 || bmp_file_header[1] != 0x4d) {
        printf("not a valid bmp file\n");
        fclose(file);
        return NULL;
    }

    uint32_t data_offset = bmp_file_header[10] | (bmp_file_header[11] << 8) |
                           (bmp_file_header[12] << 16) |
                           (bmp_file_header[13] << 24);

    uint32_t dib_header_size;
    fread(&dib_header_size, 4, 1, file);
    if (dib_header_size != 40) {
        printf("unsupported bmp version\n");
        fclose(file);
        return NULL;
    }

    fseek(file, 14, SEEK_SET);
    uint8_t dib_header[40];
    fread(dib_header, 1, 40, file);

    uint32_t width = dib_header[4] | (dib_header[5] << 8) |
                     (dib_header[6] << 16) | (dib_header[7] << 24);
    uint32_t height = dib_header[8] | (dib_header[9] << 8) |
                      (dib_header[10] << 16) | (dib_header[11] << 24);
    if (height < 1) {
        height *= -1;
    }

    uint16_t bpp = dib_header[14] | (dib_header[15] << 8);

    uint32_t pad = (width * bpp / 8 + 3) & (~3);
    uint8_t *data = malloc(pad * height);

    fseek(file, data_offset, SEEK_SET);
    fread(data, pad * height, 1, file);
    fclose(file);

    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
    for (uint32_t row = 0; row < height; row++) {
        for (uint32_t col = 0; col < width; col++) {
            uint32_t idx = (height - row - 1) * pad + col * 3;
            uint32_t pixel = (0xff << 24) | (data[idx + 2] << 16) |
                             (data[idx + 1] << 8) | data[idx];
            pixels[row * width + col] = pixel;
        }
    }

    free(data);
    return pixels;
}

// returns milliseconds since epoch
uint64_t get_time(void) {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

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

    screen_num = DefaultScreen(display);

    unsigned long border_color = BlackPixel(display, screen_num);
    unsigned long background_color = WhitePixel(display, screen_num);

    window = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                                 10, // initial x position
                                 10, // initial y position
                                 1, 1,
                                 1, // border width
                                 border_color, background_color);

    XRRScreenSize *xrrs;
    int num_sizes;
    xrrs = XRRSizes(display, 0, &num_sizes);

    Rotation current_rotation;
    XRRScreenConfiguration *conf =
        XRRGetScreenInfo(display, RootWindow(display, DefaultScreen(display)));

    int32_t original_size =
        XRRConfigCurrentConfiguration(conf, &current_rotation);
    int32_t selected_size = 6;

    XRRSetScreenConfig(display, conf,
                       RootWindow(display, DefaultScreen(display)),
                       selected_size, current_rotation, CurrentTime);

    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", true);
    Atom wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", true);
    XChangeProperty(display, window, wm_state, XA_ATOM, 32, PropModeReplace,
                    (uint8_t *)&wm_fullscreen, 1);

    gc = XCreateGC(
        display, window,
        0,     // valuemask - specifies addtional components to be set
        NULL); // values - if valuemask is set, then there we pass the data

    uint32_t *image = malloc(xrrs[selected_size].width *
                             xrrs[selected_size].height * sizeof(uint32_t));
    Buffer buffer;
    buffer.w = xrrs[selected_size].width;
    buffer.h = xrrs[selected_size].height;
    buffer.data = image;

    XImage *ximg = XCreateImage(display, DefaultVisual(display, screen_num),
                                DefaultDepth(display, screen_num), ZPixmap, 0,
                                (char *)image, xrrs[selected_size].width,
                                xrrs[selected_size].height, 32, 0);

    XStoreName(display, window, "My X11 Window!");

    XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask);

    XMapWindow(display, window);

    game_init();

    const float frame_duration = 1000.0f / FPS;

    Input input = {0};

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
                    should_quit = true;
                }

                if (key == XK_s) {
                    input.down.is_pressed = true;
                }

                if (key == XK_w) {
                    input.up.is_pressed = true;
                }

                if (key == XK_a) {
                    input.left.is_pressed = true;
                }

                if (key == XK_d) {
                    input.right.is_pressed = true;
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

        game_update_and_render(&buffer, &input, elapsed);

        XPutImage(display, window, gc, ximg, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

        input.up.was_pressed = input.up.is_pressed;
        input.down.was_pressed = input.down.is_pressed;
        input.left.was_pressed = input.left.is_pressed;
        input.right.was_pressed = input.right.is_pressed;

        uint64_t end_time = get_time();

        elapsed = (end_time - current_time) / 1000.0f;
        float sleep_time = frame_duration - elapsed * 1000;

        if (sleep_time > 0) {
            usleep(sleep_time * 1000);
        }
    }

    XRRSetScreenConfig(display, conf,
                       RootWindow(display, DefaultScreen(display)),
                       original_size, current_rotation, CurrentTime);
    XRRFreeScreenConfigInfo(conf);
    free(image);
    ximg->data = NULL;
    XDestroyImage(ximg);
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
