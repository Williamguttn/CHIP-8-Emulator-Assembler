#ifndef OPTS_H
#define OPTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    INPUT_NONE, INPUT_ASM, INPUT_HEX, INPUT_ROM
} InputMode;

typedef enum {
    ACTION_NONE, ACTION_RUN
} ActionMode;

typedef struct {
    InputMode input_mode;
    char *input_file;

    int run; // run after loading/assembling
    char *output_rom; // output ROM file
    char *output_hex; // output hex file

    int opcodes_per_frame;
    int graphics_enabled;
    int print_registers;
    int debug_enabled;

    int window_width, window_height;
} Options;

extern Options GLOB_OPTS;

Options parse_args(int argc, char **argv);

#endif // OPTS_H