#ifndef PREPROCESS_H
#define PREPROCESS_H

#include <stdint.h>
#include <stdlib.h>

char *load_file_asm(const char *filename);
int write_rom(const char *filename, const uint8_t *data, size_t size);
void write_hex(const char *filename, const uint8_t *data, size_t size);
int read_rom(const char *filename, uint8_t *data, size_t max_size);
void relocate_rom(uint8_t *rom);

#endif // PREPROCESS_H