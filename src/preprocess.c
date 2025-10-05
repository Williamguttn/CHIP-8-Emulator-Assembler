#include "preprocess.h"
#include "emulate.h"
#include "token.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

char *load_file_asm(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Could not open file: %s\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char *)malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed for file buffer.\n");
        fclose(file);
        return NULL;
    }
    size_t bytes_read = fread(buffer, 1, length, file);
    buffer[bytes_read] = '\0';
    fclose(file);
    
    // Remove comments: go line by line and delete everything after ;
    char *src = buffer;
    char *dst = buffer;
    
    while (*src) {
        if (*src == ';') {
            // Skip everything until newline
            while (*src && *src != '\n') {
                src++;
            }
            // Copy the newline if present
            if (*src == '\n') {
                *dst++ = *src++;
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    
    return buffer;
}

int write_rom(const char *filename, const uint8_t *data, size_t size) {
    if (filename == NULL || data == NULL) {
        return -1;
    }
    
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        return -1;
    }
    
    // Write the entire array to the file
    size_t bytes_written = fwrite(data, sizeof(uint8_t), size, file);
    fclose(file);
    
    if (bytes_written != size) {
        return -1;
    }
    
    return 0;
}

void write_hex(const char *filename, const uint8_t *data, size_t size) {
    if (filename == NULL || data == NULL) return;

    const size_t start = 0x200;
    if (start >= size) return; /* nothing to read */

    /* find last non-zero byte at or after start, using a signed index type */
    ptrdiff_t last = -1;
    for (ptrdiff_t i = (ptrdiff_t)size - 1; i >= (ptrdiff_t)start; --i) {
        if (data[(size_t)i] != 0) { last = i; break; }
    }
    if (last < (ptrdiff_t)start) return; /* all zeros */

    FILE *fp = fopen(filename, "w");
    if (!fp) return;

    int first = 1;
    for (size_t i = start; i <= (size_t)last; i += 2) {
        uint8_t hi = data[i];
        uint8_t lo = (i + 1 <= (size_t)last) ? data[i + 1] : 0;
        uint16_t word = ((uint16_t)hi << 8) | (uint16_t)lo;
        if (!first) fputc(' ', fp);
        fprintf(fp, "0x%04X", (unsigned)word);
        first = 0;
    }

    fputc('\n', fp);
    fclose(fp);
}

int read_rom(const char *filename, uint8_t *data, size_t max_size) {
    if (filename == NULL || data == NULL) {
        return -1;
    }
    
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return -1;
    }
    
    size_t bytes_read = fread(data, sizeof(uint8_t), max_size, file);
    fclose(file);
    
    return (int)bytes_read;
}

char *capitalize_instructions(const char *src) {
    size_t n = strlen(src);
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;

    size_t i = 0;
    while (i < n) {
        if (isalpha((unsigned char) src[i])) {
            size_t j = i;
            while (j < n && isalpha((unsigned char) src[j])) j++;
            size_t len = j - i;

            int match = 0;
            for (size_t idx = 0; INSTRUCTIONS[idx] != NULL; ++idx) {
                const char *ins = INSTRUCTIONS[idx];
                size_t ins_len = strlen(ins);
                
                if (ins_len != len) continue;

                size_t k;
                for (k = 0; k < ins_len; ++k) {
                    char csrc = src[i + k];
                    char cins = ins[k];
                    if (toupper((unsigned char) csrc) != toupper((unsigned char) cins)) break;
                }

                if (k == len) {
                    match = 1;
                    break;
                }
            }

            // convert to uppercase
            if (match) {
                for (size_t k = 0; k < len; ++k) {
                    out[i + k] = (char)toupper((unsigned char) src[i + k]);
                }
            } else {
                for (size_t k = 0; k < len; ++k) {
                    out[i + k] = src[i + k];
                }
            }
            i = j;
        } else {
            // copy as-is
            out[i] = src[i];
            ++i;
        }
    }

    out[n] = '\0';
    return out;
}

void relocate_rom(uint8_t *rom) {
    int found = 0;
    for (size_t i = 0x200; i < 0xFFF - 1; i++) {
        if ((rom[i] | rom[i + 1]) != 0x00) {
            found = 1;
            break;
        }
    }
    if (found) return;
    memmove(rom + 0x200, rom, 4096 - 0x200);
    memset(rom, 0, 0x200);
    memcpy(rom + FONTSET, fontset, FONTSET_SIZE);
}