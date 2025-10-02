#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdint.h>
#include "parse.h"

#define MAX_OPCODES 2048

typedef struct {
    uint16_t address;
    uint8_t value;
} HardValue;

typedef struct {
    uint16_t opcode;
    uint16_t address; // opcode address in memory, this value is 12 bits
} Opcode;

typedef struct {
    size_t opcode_count;
    uint16_t *opcodes;
    HardValue hard_values[MAX_OPCODES * 2]; // hardcoded byte values from DB instructions
} Codegen;


extern size_t n_hard_value;
extern HardValue temp_memory[MAX_OPCODES * 2]; // fuse opcodes & temp memory for the full memory, this will include all hardcoded bytes

Codegen codegen_ast(Node *root);

#endif // CODEGEN_H