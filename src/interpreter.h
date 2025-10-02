#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdint.h>
#include "codegen.h"

typedef struct Opcodes {
    uint16_t *data;
    HardValue hard_values[MAX_OPCODES * 2]; // hardcoded byte values from DB instructions
    size_t size;
} Opcodes;

Opcodes interpret_file_text(const char *filename);
Opcodes interpret_codegen(Codegen codegen);
void free_opcodes(Opcodes *opcodes);

#endif // INTERPRETER_H