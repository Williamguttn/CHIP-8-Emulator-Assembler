#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"

#include <stdint.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320
#define VIRTUAL_WIDTH 64
#define VIRTUAL_HEIGHT 32
//#define SCALE 8
#define SCALE (WINDOW_WIDTH / VIRTUAL_WIDTH)

extern uint8_t gfx[VIRTUAL_WIDTH][VIRTUAL_HEIGHT];
extern int draw_flag;

void init_window();
void test_draw();

#endif // GRAPHICS_H