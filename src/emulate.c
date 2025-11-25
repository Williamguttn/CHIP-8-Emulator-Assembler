#include "emulate.h"
#include "debug.h"
#include "graphics.h"
#include "opts.h"
#include "misc.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define HI(x) ((x >> 8) & 0xFF)
#define LO(x) (x & 0xFF)

#define PROM 0x200 // Program ROM
#define MEM_SIZE 0xFFF

uint8_t key[16];
uint8_t memory[4096];
uint8_t V[16];
uint16_t I;
uint16_t pc;
uint16_t stack[16];
uint8_t sp;

uint8_t delay_timer = 0;
uint8_t sound_timer = 0;

const double timer_interval = 1.0 / 60.0; // 60 Hz
double last_timer_update;

const uint8_t fontset[FONTSET_SIZE] = {
    // "0"
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    // "1"
    0x20, 0x60, 0x20, 0x20, 0x70,
    // "2"
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    // "3"
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    // "4"
    0x90, 0x90, 0xF0, 0x10, 0x10,
    // "5"
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    // "6"
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    // "7"
    0xF0, 0x10, 0x20, 0x40, 0x40,
    // "8"
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    // "9"
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    // "A"
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    // "B"
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    // "C"
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    // "D"
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    // "E"
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    // "F"
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

const uint16_t sprite_addr[0xF] = {
    0x050, 0x055, 0x05A, 0x05F,
    0x064, 0x069, 0x06E, 0x073,
    0x078, 0x07D, 0x082, 0x087,
    0x08C, 0x091, 0x096
};

int stop = 0;

void execute(uint16_t opcode) {
    uint8_t nibbles[4] = {
        (opcode >> 12) & 0xF,
        (opcode >> 8) & 0xF,
        (opcode >> 4) & 0xF,
        opcode & 0xF
    };

    uint8_t n = nibbles[3];
    uint8_t nn = nibbles[2] << 4 | n;
    uint16_t nnn = nibbles[1] << 8 | nn;
    uint8_t OP = nibbles[0];

    // Vx = NN
    if (OP == 0x6) {
        uint8_t vx = nibbles[1];
        V[vx] = nn;
    }

    // Vx = Vy
    else if (OP == 0x8 && nibbles[3] == 0x0) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        V[vx] = V[vy];
    }

    // Vx += NN
    else if (OP == 0x7) {
        uint8_t vx = nibbles[1];
        V[vx] = (V[vx] + nn) % 256;
    }

    // Vx += Vy
    else if (OP == 0x8 && nibbles[3] == 0x4) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        uint16_t sum = V[vx] + V[vy];
        V[0xF] = (sum > 0xFF) ? 1 : 0; // Set carry flag
        V[vx] = sum & 0xFF;           // Store lower 8 bits
    }

    // Vx -= Vy
    else if (OP == 0x8 && nibbles[3] == 0x5) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        V[0xF] = (V[vx] >= V[vy]) ? 1 : 0; // Set borrow flag
        V[vx] = V[vx] - V[vy];   // Store result
    }

    // Vx = Vy - Vx
    else if (OP == 0x8 && nibbles[3] == 0x7) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        V[0xF] = (V[vy] >= V[vx]) ? 1 : 0; // Set borrow flag
        V[vx] = V[vy] - V[vx];   // Store result
    }

    // Vx |= Vy
    else if (OP == 0x8 && nibbles[3] == 0x1) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        V[vx] |= V[vy];
    }

    // Vx &= Vy
    else if (OP == 0x8 && nibbles[3] == 0x2) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        V[vx] &= V[vy];
    }

    // Vx ^= Vy
    else if (OP == 0x8 && nibbles[3] == 0x3) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        V[vx] ^= V[vy];
    }

    // I = NNN
    else if (OP == 0xA) {
        I = nnn;
    }

    // I += Vx
    else if (OP == 0xF && nn == 0x1E) {
        uint8_t vx = nibbles[1];
        I += V[vx];
    }

    // goto NNN
    else if (OP == 0x1) {
        pc = nnn;
    }

    // if (Vx == NN) skip next instruction
    else if (OP == 0x3) {
        uint8_t vx = nibbles[1];
        if (V[vx] == nn) {
            pc += 2;
        }
    }

    // if (Vx != NN) skip next instruction
    else if (OP == 0x4) {
        uint8_t vx = nibbles[1];
        if (V[vx] != nn) {
            pc += 2;
        }
    }

    // if (Vx != Vy) skip next instruction
    else if (OP == 0x9 && nibbles[3] == 0x0) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        if (V[vx] != V[vy]) {
            pc += 2;
        }
    }

    // if (Vx == Vy) skip next instruction
    else if (OP == 0x5 && nibbles[3] == 0x0) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        if (V[vx] == V[vy]) {
            pc += 2;
        }
    }

    // call NNN
    else if (OP == 0x2) {
        stack[sp++] = pc;
        pc = nnn;
    }

    // return from subroutine
    else if (((nibbles[0] | nibbles[1]) == 0x00) && nn == 0xEE) {
        pc = stack[--sp];
    }

    // draw(Vx, Vy, N)
    else if (OP == 0xD) {
        uint8_t vx = nibbles[1];
        uint8_t vy = nibbles[2];
        uint8_t height = nibbles[3];
        V[0xF] = 0; // Reset collision flag

        for (int row = 0; row < height; row++) {
            uint8_t sprite = memory[I + row];
            for (int col = 0; col < 8; col++) {
                if (sprite & (0x80 >> col)) { // Check if the current bit is set
                    int x = (V[vx] + col) % VIRTUAL_WIDTH;
                    int y = (V[vy] + row) % VIRTUAL_HEIGHT;

                    if (gfx[x][y] == 1) {
                        V[0xF] = 1; // Set collision flag
                    }
                    gfx[x][y] ^= 1; // XOR pixel
                }
            }
        }
        draw_flag = 1;
    }

    // clear_screen()
    else if (opcode == 0x00E0) {
        memset(gfx, 0, sizeof(gfx));
        draw_flag = 1;
    }

    // SKP Vx
    else if (OP == 0xE && nn == 0x9E) {
        uint8_t vx = nibbles[1];
        if (key[V[vx]]) {
            pc += 2;
        }
    }

    // SKNP Vx
    else if (OP == 0xE && nn == 0xA1) {
        uint8_t vx = nibbles[1];
        if (!key[V[vx]]) {
            pc += 2;
        }
    }

    // reg_dump(Vx, &I)
    else if (OP == 0xF && nn == 0x55) {
        uint8_t vx = nibbles[1];
        for (int i = 0; i <= vx; i++) {
            memory[I + i] = V[i];
        }
    }

    // reg_load(Vx, &I)
    else if (OP == 0xF && nn == 0x65) {
        uint8_t vx = nibbles[1];
        for (int i = 0; i <= vx; i++) {
            V[i] = memory[I + i];
        }
    }

    // delay_timer(Vx)
    else if (OP == 0xF && nn == 0x15) {
        uint8_t vx = nibbles[1];
        delay_timer = V[vx];
    }

    // sound_timer(Vx)
    else if (OP == 0xF && nn == 0x18) {
        uint8_t vx = nibbles[1];
        sound_timer = V[vx];
    }
    
    // Vx = get_delay()
    else if (OP == 0xF && nn == 0x07) {
        uint8_t vx = nibbles[1];
        V[vx] = delay_timer;
    }

    // I = sprite_addr[Vx]
    else if (OP == 0xF && nn == 0x29) {
        uint8_t vx = nibbles[1];
        I = sprite_addr[V[vx] & 0x0F];
    }

    // I += Vx
    else if (OP == 0xF && nn == 0x1E) {
        uint8_t vx = nibbles[1];
        I += V[vx];
    }

    // BCD
    else if (OP == 0xF && nn == 0x33) {
        uint8_t vx = nibbles[1];
        memory[I] = V[vx] / 100;
        memory[I + 1] = (V[vx] / 10) % 10;
        memory[I + 2] = V[vx] % 10;
    }

    // Vx = rand() & NN
    else if (OP == 0xC) {
        uint8_t vx = nibbles[1];
        V[vx] = rand() & nn;
    }

    // SHR Vx
    else if (OP == 0x8 && nibbles[3] == 0x6) {
        uint8_t vx = nibbles[1];
        V[0xF] = V[vx] & 0x1;
        V[vx] >>= 1;
    }
    // SHL Vx
    else if (OP == 0x8 && nibbles[3] == 0xE) {
        uint8_t vx = nibbles[1];
        V[0xF] = V[vx] >> 7;
        V[vx] <<= 1;
    }

    else {
        if (GLOB_OPTS.debug_enabled)
            printf("Unknown opcode: 0x%04X\n.", opcode);
        //stop = 1; // Stop execution on unknown opcode
    }
}

void timer_tick() {
    double now = get_time_seconds();
    if (now - last_timer_update >= timer_interval) {
        if (delay_timer > 0) delay_timer--;
        if (sound_timer > 0) sound_timer--;

        last_timer_update += timer_interval;
    }
}

void start_no_graphics() {
    while (!stop) {
        uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];

        /*if (opcode == 0x0000) { // debug
            printf("Reached empty opcode at PC: 0x%04X. Stopping execution.\n", pc);
            stop = 1;
            break;
        }*/

        pc += 2;
        execute(opcode);

        timer_tick();
    }
}

void step_emulator() {
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
    pc += 2;
    execute(opcode);
}

void start_emulation() {
    last_timer_update = get_time_seconds();
    if (GLOB_OPTS.graphics_enabled) {
        init_window(); // Emulation happens in steps with the graphics loop
    } else {
        start_no_graphics();
    }

    if (GLOB_OPTS.print_registers || GLOB_OPTS.debug_enabled)
        print_registers();

    if (GLOB_OPTS.debug_enabled) {
        memory_dump(0x200, 0x300, memory);
        memory_dump(FONTSET, FONTSET + FONTSET_SIZE + 10, memory);
    }
}

void init_emulator(Opcodes opcodes_struct, uint8_t *pbp_memory, int rom_only) {
    size_t n_opcodes = opcodes_struct.size;
    uint16_t *opcodes = opcodes_struct.data;
    HardValue *hard_values = opcodes_struct.hard_values;

    if (n_opcodes > (MEM_SIZE - PROM)) {
        fprintf(stderr, "Error: Too many opcodes to fit in memory\n");
        return;
    }

    memset(memory, 0, sizeof(memory));

    // load fontset
    memcpy(&memory[FONTSET], fontset, sizeof(fontset));

    memset(V, 0, sizeof(V));
    memset(stack, 0, sizeof(stack));
    I = 0;
    pc = PROM; // Program counter starts at 0x200

    for (size_t i = 0; i < n_opcodes; i++) {
        memory[PROM + i * 2] = HI(opcodes[i]);     // High byte
        memory[PROM + i * 2 + 1] = LO(opcodes[i]); // Low byte
    }

    // after opcodes are done, we will insert the hard values and shift everything to the right
    // labels and addresses are already adjusted for this
    for (size_t i = 0; i < n_hard_value; i++) {
        HardValue hard_val = hard_values[i];
        if (hard_val.address < MEM_SIZE) {
            // shift all values in memory to the right of hard_val.address by 1
            for (size_t j = MEM_SIZE - 1; j > hard_val.address; j--) {
                memory[j] = memory[j - 1];
            }
            memory[hard_val.address] = hard_val.value;
        } else {
            fprintf(stderr, "Warning: Hard value address 0x%04X out of bounds, skipping\n", hard_val.address);
        }
    }

    // copy memory to pbp_memory
    memcpy(pbp_memory, memory, sizeof(memory));
    if (rom_only) {
        return;   
    }

    start_emulation();
}

void init_emulator_rom(uint8_t *rom) {
    I = 0;
    pc = PROM;
    memset(V, 0, sizeof(V));
    memset(stack, 0, sizeof(stack));
    memcpy(memory, rom, sizeof(memory));

    start_emulation();
}