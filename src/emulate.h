#ifndef EMULATE_H
#define EMULATE_H

#include <stdint.h>
#include "interpreter.h"

#define FONTSET_SIZE 80
#define FONTSET 0x50

extern const uint8_t fontset[FONTSET_SIZE];
extern uint8_t key[16];
extern uint8_t memory[4096];
extern uint8_t V[16];
extern uint16_t I;
extern uint16_t pc;
extern uint16_t stack[16];
extern uint8_t sp;

void timer_tick();
void init_emulator(Opcodes opcodes_struct, uint8_t *pbp_memory, int rom_only);
void init_emulator_rom(uint8_t *rom);
void step_emulator();

#endif // EMULATE_H