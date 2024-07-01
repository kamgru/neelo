#include "game.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define FPS 60

void platform_test(char *arg) { printf("[platform]: %s\n", arg); }

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

  uint32_t *image = malloc(SCREEN_W * SCREEN_H * sizeof(uint32_t));
  Buffer buffer;
  buffer.w = SCREEN_W;
  buffer.h = SCREEN_H;
  buffer.data = image;

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

  free(image);
  ximg->data = NULL;
  XDestroyImage(ximg);
  XCloseDisplay(display);
  return 0;
}
