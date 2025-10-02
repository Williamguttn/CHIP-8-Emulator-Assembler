#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

void memory_dump(int start, int end, uint8_t *memory);
void print_registers();

#endif //DEBUG_H