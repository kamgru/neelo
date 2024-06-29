#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_W 1024
#define SCREEN_H 768
#define CORNFLOWER_BLUE 0x6495ed
#define LIGHT_SALMON 0xffa07a
#define YELLOW_GREEN 0x9acd32

void fill_color(uint32_t *image, uint32_t color)
{

    for (int i = 0; i < SCREEN_W * SCREEN_H; i++)
    {
        image[i] = color;
    }
}

void draw_rect(uint32_t* buffer, uint32_t color, uint16_t w, uint16_t h, uint16_t x, uint16_t y) {
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++){
			buffer[(y + j) * SCREEN_W + (x + i)] = color;
		}
	}
}

int main(void)
{
    Display *display;
    Window window;
    XEvent evt;
    GC gc;
    const char *msg = "Hello, World!";
    int screen_num;

    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
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

    gc = XCreateGC(display, window,
                   0,     // valuemask - specifies addtional components to be set
                   NULL); // values - if valuemask is set, then there we pass the data

    XImage *ximg = XCreateImage(display, DefaultVisual(display, screen_num), DefaultDepth(display, screen_num), ZPixmap,
                                0, (char *)image, SCREEN_W, SCREEN_H, 32, 0);

    XStoreName(display, window, "My X11 Window!");

    XSelectInput(display, window, ExposureMask | KeyPressMask);

    XMapWindow(display, window);

    while (1)
    {
        XNextEvent(display, &evt);

        if (evt.type == KeyPress)
        {
            if (evt.xkey.keycode == 0x9)
            {
                break;
            }
            if (evt.xkey.keycode == 0xa)
            {
                fill_color(image, CORNFLOWER_BLUE);
				draw_rect(image, YELLOW_GREEN, 100, 100, 500, 350);
            }
            if (evt.xkey.keycode == 0xb)
            {
                fill_color(image, LIGHT_SALMON);
				draw_rect(image, CORNFLOWER_BLUE, 300, 50, 3, 600);
            }
            if (evt.xkey.keycode == 0xc)
            {
                fill_color(image, YELLOW_GREEN);
				draw_rect(image, LIGHT_SALMON, 20, 20, 10, 10);
            }
        }
    	XPutImage(display, window, gc, ximg, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    }

    XDestroyImage(ximg);
    XCloseDisplay(display);
    free(image);
    return 0;
}
