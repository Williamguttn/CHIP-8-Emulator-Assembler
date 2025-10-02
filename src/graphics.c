#include "graphics.h"
#include "emulate.h"
#include "opts.h"

#include <math.h>
#include <stdio.h>


//#define OPCODES_PER_FRAME 50

/*
Keypad     Keyboard
1 2 3 C    1 2 3 4    
4 5 6 D    Q W E R
7 8 9 E    A S D F
A 0 B F    Z X C V
*/
int keys[16] = {
    // 0x0, 0x1, 0x2, 0x3
    KEY_X, KEY_ONE, KEY_TWO, KEY_THREE,
    // 0x4, 0x5, 0x6, 0x7
    KEY_Q, KEY_W, KEY_E, KEY_A,
    // 0x8, 0x9, 0xA, 0xB
    KEY_S, KEY_D, KEY_Z, KEY_C,
    // 0xC, 0xD, 0xE, 0xF
    KEY_FOUR, KEY_R, KEY_F, KEY_V
};

uint8_t gfx[VIRTUAL_WIDTH][VIRTUAL_HEIGHT];
int draw_flag = 0;

RenderTexture2D screen;

void drawPixelOnTexture(RenderTexture2D target, int x, int y, Color color) {
    BeginTextureMode(target);
    DrawRectangle(x, y, 1, 1, color);
    EndTextureMode();
}

// Emulation loop called every frame
void emulation_loop() {
    // check all keys

    for (int i = 0; i < 16; i++) {
        if (IsKeyDown(keys[i])) {
            key[i] = 1;
        } else {
            key[i] = 0;
        }
    }

    for (int i = 0; i < GLOB_OPTS.opcodes_per_frame; i++) {
        step_emulator();
    }
    timer_tick();

    if (draw_flag) {
        BeginTextureMode(screen);
        ClearBackground(BLACK);
        for (int y = 0; y < VIRTUAL_HEIGHT; y++) {
            for (int x = 0; x < VIRTUAL_WIDTH; x++) {
                if (gfx[x][y]) {
                    DrawRectangle(x, y, 1, 1, WHITE);
                }
            }
        }
        EndTextureMode();
        draw_flag = false;
    }

    BeginDrawing();
    ClearBackground(DARKGRAY);

    DrawTexturePro(
        screen.texture,
        (Rectangle){ 0, 0, VIRTUAL_WIDTH, -VIRTUAL_HEIGHT },
        (Rectangle){ 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    EndDrawing();
}

void init_window() {
    printf("\n\n");
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CHIP-8 Emulator");
    screen = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

    SetTextureFilter(screen.texture, TEXTURE_FILTER_POINT);
    SetTargetFPS(60);

    BeginTextureMode(screen);
    ClearBackground(BLACK);
    EndTextureMode();

    while (!WindowShouldClose()) {
        emulation_loop();
    }
}