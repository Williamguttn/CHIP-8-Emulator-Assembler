// reads opcodes from file and interprets them

#include <stdio.h>

#include "emulate.h"
#include "interpreter.h"
#include "codegen.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Opcodes interpret_file_text(const char *filename) {
    Opcodes res = {NULL, {0}, 0};
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return res;
    }

    size_t capacity = 16;
    uint16_t *opcodes = malloc(capacity * sizeof(uint16_t));
    size_t count = 0;
    char line[1024];

    while (fgets(line, sizeof(line), file)) {
        char *p = line;
        while (*p) {
            if (*p == '#') break;
            if (strncmp(p, "0x", 2) == 0) {
                uint16_t opcode;
                if (sscanf(p, "0x%4hx", &opcode) == 1) {
                    if (count >= capacity) {
                        capacity *= 2;
                        uint16_t *temp = realloc(opcodes, capacity * sizeof(uint16_t));
                        if (!temp) {
                            perror("Failed to allocate memory");
                            fclose(file);
                            free(opcodes);
                            return res;
                        }
                        opcodes = temp;
                    }
                    opcodes[count++] = opcode;
                    p += 5;
                }
            }
            p++;
        }
    }

    fclose(file);
    res.data = opcodes;
    res.size = count;
    return res;
}

Opcodes interpret_codegen(Codegen codegen) {
    Opcodes res = {NULL, {0}, 0};
    res.data = malloc(codegen.opcode_count * sizeof(uint16_t));
    if (!res.data) {
        perror("Failed to allocate memory for opcodes");
        return res;
    }
    memcpy(res.data, codegen.opcodes, codegen.opcode_count * sizeof(uint16_t));
    memcpy(res.hard_values, codegen.hard_values, (MAX_OPCODES * 2) * sizeof(HardValue));
    res.size = codegen.opcode_count;
    return res;
}

void free_opcodes(Opcodes *opcodes) {
    if (opcodes->data) {
        free(opcodes->data);
        opcodes->data = NULL;
    }
    opcodes->size = 0;
}