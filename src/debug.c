#include "debug.h"
#include "emulate.h"

#include <stdio.h>

void memory_dump(int start, int end, uint8_t *memory) {
    printf("\n=== Memory Dump (0x%03X - 0x%03X) ===\n", start, end);
    
    // Align start to 16-byte boundary for cleaner output
    int aligned_start = start & ~0xF;
    
    for (int addr = aligned_start; addr < end; addr += 16) {
        // Skip rows entirely before start address
        if (addr + 16 <= start) continue;
        
        printf("0x%04X: ", addr);
        
        // Print hex values
        for (int i = 0; i < 16; i++) {
            int current_addr = addr + i;
            if (current_addr < start || current_addr >= end) {
                printf("   ");
            } else {
                printf("%02X ", memory[current_addr]);
            }
            if (i == 7) printf(" "); // Extra space in the middle
        }
        
        printf(" | ");
        
        // Print ASCII representation
        for (int i = 0; i < 16; i++) {
            int current_addr = addr + i;
            if (current_addr >= start && current_addr < end) {
                uint8_t byte = memory[current_addr];
                printf("%c", (byte >= 32 && byte <= 126) ? byte : '.');
            } else {
                printf(" ");
            }
        }
        
        printf("\n");
    }
    
    printf("================================\n");
}

void print_registers() {
    printf("Registers:\n");
    for (int i = 0; i < 16; i++) {
        printf("V[%X]: 0x%02X\n", i, V[i]);
    }
    printf("I: 0x%04X\n", I);
    printf("PC: 0x%04X\n", pc);
    printf("SP: 0x%02X\n", sp);
}