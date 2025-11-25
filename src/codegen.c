#include "codegen.h"
#include "symbtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

static Opcode opcodes[MAX_OPCODES];
uint16_t final_opcodes[MAX_OPCODES];

size_t n_hard_value = 0;
HardValue temp_memory[MAX_OPCODES * 2]; // fuse opcodes & temp memory for the full memory, this will include all hardcoded bytes

symtab_t *label_registry = NULL; // label symbol table
Codegen codegen;

// this helps determine the location of labels
uint16_t addr_ptr = 0x200;

const char *registers[17] = {
    "V0", "V1", "V2", "V3", "V4", "V5", "V6", "V7",
    "V8", "V9", "VA", "VB", "VC", "VD", "VE", "VF", "I"
};

const char *timers[2] = {"DT", "ST"};

typedef struct {
    char *label;
    uint16_t opcode_index;
} PendingRef;

static PendingRef *pending_refs = NULL;
static size_t pending_count = 0;
static size_t pending_capacity = 0;

static char *strdup_local(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

static int pending_ensure_capacity(void) {
    if (pending_count < pending_capacity) return 0;
    size_t newcap = pending_capacity ? pending_capacity * 2 : 8;
    PendingRef *tmp = (PendingRef *)realloc(pending_refs, newcap * sizeof(PendingRef));
    if (!tmp) return -1;
    pending_refs = tmp;
    pending_capacity = newcap;
    return 0;
}

static int pending_add(const char *label, uint16_t opcode_index) {
    if (pending_ensure_capacity() != 0) return -1;
    char *copy = strdup_local(label);
    if (!copy) return -1;
    pending_refs[pending_count].label = copy;
    pending_refs[pending_count].opcode_index = opcode_index;
    pending_count += 1;
    return 0;
}

static void pending_resolve(const char *label, uint16_t label_addr) {
    size_t i = 0;
    while (i < pending_count) {
        if (strcmp(pending_refs[i].label, label) == 0) {
            uint16_t idx = pending_refs[i].opcode_index;
            if (idx < MAX_OPCODES) {
                // patch the opcode slot to have the correct low 12 bits
                opcodes[idx].opcode = (uint16_t)(opcodes[idx].opcode & 0xF000) | (label_addr & 0x0FFF);
            } else {
                fprintf(stderr, "Internal: pending opcode_index out of bounds: %u\n", idx);
            }
            free(pending_refs[i].label);
            pending_count -= 1;
            if (i != pending_count) {
                pending_refs[i] = pending_refs[pending_count];
            }
        } else {
            i += 1;
        }
    }
}

static void pending_clear_all(void) {
    for (size_t i = 0; i < pending_count; ++i) {
        free(pending_refs[i].label);
    }
    free(pending_refs);
    pending_refs = NULL;
    pending_count = 0;
    pending_capacity = 0;
}

void add_opcode(uint16_t opcode) {
    if (codegen.opcode_count < MAX_OPCODES) {
        Opcode new_opcode = {  .opcode = opcode, .address = addr_ptr };
        opcodes[codegen.opcode_count++] = new_opcode;
        addr_ptr += 2;
        //opcodes[codegen.opcode_count++] = opcode;
    } else {
        fprintf(stderr, "Opcode buffer overflow\n");
    }
}

void add_label(const char *name, uint16_t addr) {
    symtab_put(label_registry, name, addr);
}

int get_label_address(const char *name, uint16_t *out_addr) {
    return symtab_get(label_registry, name, out_addr);
}

int8_t is_register(const char *value) {
    for (int i = 0; i < 17; i++) {
        if (strcmp(value, registers[i]) == 0) {
            return (int8_t)i;
        }
    }
    return -1;
}

int8_t is_timer(const char *value) {
    for (int i = 0; i < 2; i++) {
        if (strcmp(value, timers[i]) == 0) {
            return (int8_t)i;
        }
    }
    return -1;
}

// reg to field
static inline uint16_t rf(uint8_t r, int shift) {
    return (uint16_t)r << shift;
}

uint16_t nn(const char *value) {
    if (value == NULL) {
        return 0;
    }

    while (isspace((unsigned char)*value)) {
        value++;
    }

    if (*value == '\0') {
        return 0;
    }
    
    int is_hex = 0;
    if (value[0] == '0' && (value[1] == 'x' || value[1] == 'X')) {
        is_hex = 1;
        value += 2;
    }
    
    uint32_t result = 0;
    int digit_count = 0;
    
    if (is_hex) {
        while (*value != '\0') {
            char c = tolower((unsigned char)*value);
            
            if (c >= '0' && c <= '9') {
                result = result * 16 + (c - '0');
            } else if (c >= 'a' && c <= 'f') {
                result = result * 16 + (c - 'a' + 10);
            } else {
                break;
            }
            
            digit_count++;
            value++;
            
            if (result > 0xFF && digit_count > 2) {
                return 0x00FF;
            }
        }
    } else {
        while (*value != '\0') {
            if (*value >= '0' && *value <= '9') {
                result = result * 10 + (*value - '0');
                digit_count++;
                
                if (result > 0xFF && digit_count > 3) {
                    return 0x00FF;
                }
            } else {
                break;
            }
            value++;
        }
    }
    
    if (result > 0xFF) {
        return 0x00FF;
    }
    
    return (uint16_t)result;
}

void evaluate_deref_LD(Node *node) {
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    // LD Vx, [I]
    if (arg2->type == NODE_DEREFERENCE) {
        int vx = is_register(arg1->value);
        if (vx == -1) {
            fprintf(stderr, "First argument of LD must be a register, got %s\n", arg1->value);
            return;
        }
        if (strcmp(arg2->value, "I") != 0) {
            fprintf(stderr, "Dereferenced argument must be I, got %s\n", arg2->value);
            return;
        }
        uint16_t opcode = 0xF065 | rf(vx, 8);
        add_opcode(opcode);
        return;
    }

    // LD [I], Vx
    if (arg1->type == NODE_DEREFERENCE) {
        int vx = is_register(arg2->value);
        if (vx == -1) {
            fprintf(stderr, "Second argument of LD must be a register, got %s\n", arg2->value);
            return;
        }
        if (strcmp(arg1->value, "I") != 0) {
            fprintf(stderr, "Dereferenced argument must be I, got %s\n", arg1->value);
            return;
        }
        uint16_t opcode = 0xF055 | rf(vx, 8);
        add_opcode(opcode);
        return;
    }
}

void evaluate_timer_LD(Node *node) {
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    int t1 = is_timer(arg1->value);
    int t2 = is_timer(arg2->value);

    // LD DT/ST, Vx
    if (t1 != -1) {
        int vx = is_register(arg2->value);
        if (vx == -1) {
            fprintf(stderr, "Second argument of LD must be a register, got %s\n", arg2->value);
            return;
        }
        if (strcmp(arg1->value, "DT") == 0) {
            // LD DT, Vx
            uint16_t opcode = 0xF015 | rf(vx, 8);
            add_opcode(opcode);
        } else {
            // LD ST, Vx
            uint16_t opcode = 0xF018 | rf(vx, 8);
            add_opcode(opcode);
        }
        return;
    }

    // LD Vx, DT
    if (t2 != -1) {
        int vx = is_register(arg1->value);
        if (vx == -1) {
            fprintf(stderr, "First argument of LD must be a register, got %s\n", arg1->value);
            return;
        }
        if (strcmp(arg2->value, "DT") == 0) {
            // LD Vx, DT
            uint16_t opcode = 0xF007 | rf(vx, 8);
            add_opcode(opcode);
        }
        return;
    }
}

void evaluate_bcd_LD(Node *node) {
    Node *arg2 = node->args[1];

    // LD B, Vx
    int vx = is_register(arg2->value);
    if (vx == -1) {
        fprintf(stderr, "Second argument of LD must be a register, got %s\n", arg2->value);
        return;
    }
    uint16_t opcode = 0xF033 | rf(vx, 8);
    add_opcode(opcode);
}

void evaluate_LD(Node *node) {
    if (node->n_args != 2) {
        fprintf(stderr, "Expected 2 arguments for LD, got %d\n", node->n_args);
        return;
    }

    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    // LD [I], Vx / LD Vx, [I]
    if (arg1->type == NODE_DEREFERENCE || arg2->type == NODE_DEREFERENCE) {
        evaluate_deref_LD(node);
        return;
    }

    // LD DT/ST, Vx
    // LD Vx, DT
    if (is_timer(arg1->value) != -1 || is_timer(arg2->value) != -1) {
        evaluate_timer_LD(node);
        return;
    }

    // LD B, Vx (BCD)
    if (strcmp(arg1->value, "B") == 0) {
        evaluate_bcd_LD(node);
        return;
    }

    int vx = is_register(arg1->value); // Returns register index or -1
    int ireg = (strcmp(arg1->value, "I") == 0) ? 1 : 0;

    if (vx == -1) {
        fprintf(stderr, "First argument of LD must be a register, got %s\n", arg1->value);
        return;
    }

    if (arg2->type == NODE_IDENTIFIER && is_register(arg2->value) == -1 && !ireg) {
        fprintf(stderr, "Second argument of LD must be a register or number, got identifier %s\n", arg2->value);
        return;
    }

    if (ireg) {  
        // LD I, NNN/Vx
        if (strcmp(arg1->value, "I") == 0) {
            int vx = is_register(arg2->value);

            if (vx != -1) { // LD I, Vx
                // LD I, Vx (used for fontsets)
                uint16_t opcode = 0xF029 | rf(vx, 8);
                add_opcode(opcode);
                return;
            }
            // LD I, NNN
            if (arg2->type == NODE_IDENTIFIER) {
                uint16_t address;
                if (get_label_address(arg2->value, &address)) {
                    uint16_t opcode = 0xA000 | (address & 0x0FFF);
                    add_opcode(opcode);
                } else {
                    size_t slot_index = codegen.opcode_count;
                    add_opcode((uint16_t)(0xA000)); // placeholder, will patch later
                    if (pending_add(arg2->value, (uint16_t)slot_index) != 0) {
                        fprintf(stderr, "Failed to add pending reference for label %s\n", arg2->value);
                        return;
                    }
                }
                return;
            } else if (arg2->type == NODE_NUMBER) {
                uint16_t address = nn(arg2->value);
                uint16_t opcode = 0xA000 | address;
                add_opcode(opcode);
                return;
            } else {
                fprintf(stderr, "LD I expects a number or identifier as second argument, got type %d\n", arg2->type);
                return;
            }
        }
    } else {
        int vy = is_register(arg2->value);

        if (vy != -1) {
            // LD Vx, Vy
            uint16_t opcode = 0x8000 | rf(vx, 8) | rf(vy, 4);
            add_opcode(opcode);
            return;
        }

        // LD Vx, NN
        uint16_t value = nn(arg2->value);
        uint16_t opcode = 0x6000 | rf(vx, 8) | value;
        add_opcode(opcode);
    }
}

void evaluate_JP(Node *node) {
    if (node->n_args != 1) {
        fprintf(stderr, "Expected 1 argument for JP, got %d\n", node->n_args);
        return;
    }

    Node *arg1 = node->args[0];

    if (arg1->type == NODE_IDENTIFIER) {
        uint16_t address;
        if (get_label_address(arg1->value, &address)) {
            uint16_t opcode = 0x1000 | (address & 0x0FFF);
            add_opcode(opcode);
        } else {
            size_t slot_index = codegen.opcode_count;
            add_opcode((uint16_t)(0x1000)); // placeholder, will patch later
            if (pending_add(arg1->value, (uint16_t)slot_index) != 0) {
                fprintf(stderr, "Failed to add pending reference for label %s\n", arg1->value);
                return;
            }
        }
    } else if (arg1->type == NODE_NUMBER) {
        uint16_t address = nn(arg1->value);
        uint16_t opcode = 0x1000 | address;
        add_opcode(opcode);
    } else {
        fprintf(stderr, "JP instruction expects a number or identifier as argument, got type %d\n", arg1->type);
        return;
    }
}

void evaluate_ADD(Node *node) {
    if (node->n_args != 2) {
        fprintf(stderr, "Expected 2 arguments for ADD, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    int vx = is_register(arg1->value);
    int ireg = (strcmp(arg1->value, "I") == 0) ? 1 : 0;

    if (vx == -1 && !ireg) {
        fprintf(stderr, "First argument of ADD must be a register, got %s\n", arg1->value);
        return;
    }

    int vy = is_register(arg2->value);

    if (ireg) {
        if (vy == -1) {
            fprintf(stderr, "Second argument of ADD must be a register, got %s\n", arg2->value);
            return;
        }

        // ADD I, Vy
        uint16_t opcode = 0xF01E | rf(vy, 8);
        add_opcode(opcode);

        return;
    }

    if (vy != -1) {
        // ADD Vx, Vy
        uint16_t opcode = 0x8004 | rf(vx, 8) | rf(vy, 4);
        add_opcode(opcode);
    } else {
        // ADD Vx, NN
        uint16_t value = nn(arg2->value);
        uint16_t opcode = 0x7000 | rf(vx, 8) | value;
        add_opcode(opcode);
    }
}

void evaluate_SUB(Node *node) {
    if (node->n_args != 2) {
        fprintf(stderr, "Expected 2 arguments for SUB, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    int vx = is_register(arg1->value);
    int vy = is_register(arg2->value);

    if (vx == -1 || vy == -1) {
        fprintf(stderr, "%s argument of SUB must be a register, got %s\n", (vx == -1 ? "First" : "Second"), arg1->value);
        return;
    }

    uint16_t opcode = 0x8005 | rf(vx, 8) | rf(vy, 4);
    add_opcode(opcode);
}

void evaluate_SUBN(Node *node) {
    if (node->n_args != 2) {
        fprintf(stderr, "Expected 2 arguments for SUBN, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    int vx = is_register(arg1->value);
    int vy = is_register(arg2->value);

    if (vx == -1 || vy == -1) {
        fprintf(stderr, "%s argument of SUBN must be a register, got %s\n", (vx == -1 ? "First" : "Second"), arg1->value);
        return;
    }

    uint16_t opcode = 0x8007 | rf(vx, 8) | rf(vy, 4);
    add_opcode(opcode);
}

void evaluate_SE(Node *node) {
    if (node->n_args != 2) {
        fprintf(stderr, "Expected 2 arguments for SE, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    int vx = is_register(arg1->value); // Returns register index or -1

    if (vx == -1) {
        fprintf(stderr, "First argument of SE must be a register, got %s\n", arg1->value);
        return;
    }

    if (arg2->type == NODE_IDENTIFIER && is_register(arg2->value) == -1) {
        fprintf(stderr, "Second argument of SE must be a register or number, got identifier %s\n", arg2->value);
        return;
    }

    int vy = is_register(arg2->value);

    if (vy != -1) {
        // SE Vx, Vy
        uint16_t opcode = 0x5000 | rf(vx, 8) | rf(vy, 4);
        add_opcode(opcode);
    } else {
        // SE Vx, NN
        uint16_t value = nn(arg2->value);
        uint16_t opcode = 0x3000 | rf(vx, 8) | value;
        add_opcode(opcode);
    }
}

void evaluate_SNE(Node *node) {
    if (node->n_args != 2) {
        fprintf(stderr, "Expected 2 arguments for SNE, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    int vx = is_register(arg1->value); // Returns register index or -1

    if (vx == -1) {
        fprintf(stderr, "First argument of SNE must be a register, got %s\n", arg1->value);
        return;
    }

    if (arg2->type == NODE_IDENTIFIER && is_register(arg2->value) == -1) {
        fprintf(stderr, "Second argument of SNE must be a register or number, got identifier %s\n", arg2->value);
        return;
    }

    int vy = is_register(arg2->value);

    if (vy != -1) {
        // SNE Vx, Vy
        uint16_t opcode = 0x9000 | rf(vx, 8) | rf(vy, 4);
        add_opcode(opcode);
    } else {
        // SNE Vx, NN
        uint16_t value = nn(arg2->value);
        uint16_t opcode = 0x4000 | rf(vx, 8) | value;
        add_opcode(opcode);
    }
}

void evaluate_CALL(Node *node) {
    if (node->n_args != 1) {
        fprintf(stderr, "Expected 1 argument for CALL, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];
    if (arg1->type == NODE_IDENTIFIER) {
        uint16_t address;
        if (get_label_address(arg1->value, &address)) {
            uint16_t opcode = 0x2000 | (address & 0x0FFF);
            add_opcode(opcode);
        } else {
            size_t slot_index = codegen.opcode_count;
            add_opcode((uint16_t)(0x2000));
            if (pending_add(arg1->value, (uint16_t)slot_index) != 0) {
                fprintf(stderr, "Failed to add pending reference for label %s\n", arg1->value);
                return;
            }
        }
    } else {
        fprintf(stderr, "CALL instruction expects an identifier as argument, got type %d\n", arg1->type);
        return;
    }
}

void evaluate_RET(Node *node) {
    if (node->n_args != 0) {
        fprintf(stderr, "Expected 0 arguments for RET, got %d\n", node->n_args);
        return;
    }
    uint16_t opcode = 0x00EE;
    add_opcode(opcode);
}

void evaluate_DB(Node *node) {
    if (node->n_args < 1) {
        fprintf(stderr, "Expected at least 1 argument for DB, got %d\n", node->n_args);
        return;
    }
    for (int i = 0; i < node->n_args; i++) {
        Node *arg = node->args[i];
        if (arg->type != NODE_NUMBER) {
            fprintf(stderr, "DB instruction expects number arguments, got type %d\n", arg->type);
            continue;
        }
        uint16_t value = nn(arg->value);
        if (value > 0xFF) {
            fprintf(stderr, "DB argument out of range (0-255), got %u\n", value);
            value = 0xFF;
        }

        HardValue hard_val = { .address = addr_ptr++, .value = (uint8_t)(value & 0xFF) };
        temp_memory[n_hard_value++] = hard_val;
    }
}

void evaluate_DRW(Node *node) {
    if (node->n_args != 3) {
        fprintf(stderr, "Expected 3 arguments for DRW, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];
    Node *arg3 = node->args[2];

    int vx = is_register(arg1->value); // Returns register index or -1
    int vy = is_register(arg2->value); // Returns register index or -1

    if (vx == -1) {
        fprintf(stderr, "First argument of DRW must be a register, got %s\n", arg1->value);
        return;
    }

    if (vy == -1) {
        fprintf(stderr, "Second argument of DRW must be a register, got %s\n", arg2->value);
        return;
    }

    if (arg3->type != NODE_NUMBER) {
        fprintf(stderr, "Third argument of DRW must be a number, got type %d\n", arg3->type);
        return;
    }

    uint16_t n = nn(arg3->value);
    if (n > 0xF) {
        fprintf(stderr, "Third argument of DRW out of range (0-15), got %u\n", n);
        n = 0xF;
    }

    uint16_t opcode = 0xD000 | rf(vx, 8) | rf(vy, 4) | (n & 0x000F);
    add_opcode(opcode);
}

void evaluate_CLS(Node *node) {
    if (node->n_args != 0) {
        fprintf(stderr, "Expected 0 arguments for CLS, got %d\n", node->n_args);
        return;
    }
    uint16_t opcode = 0x00E0;
    add_opcode(opcode);
}

void evaluate_SKP(Node *node) {
    if (node->n_args != 1) {
        fprintf(stderr, "Expected 1 argument for SKP, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];

    int vx = is_register(arg1->value); // Returns register index or -1

    if (vx == -1) {
        fprintf(stderr, "Argument of SKP must be a register, got %s\n", arg1->value);
        return;
    }

    uint16_t opcode = 0xE09E | rf(vx, 8);
    add_opcode(opcode);
}

void evaluate_SKNP(Node *node) {
    if (node->n_args != 1) {
        fprintf(stderr, "Expected 1 argument for SKNP, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];

    int vx = is_register(arg1->value); // Returns register index or -1

    if (vx == -1) {
        fprintf(stderr, "Argument of SKNP must be a register, got %s\n", arg1->value);
        return;
    }
    uint16_t opcode = 0xE0A1 | rf(vx, 8);
    add_opcode(opcode);
}

void evaluate_XOR(Node *node) {
    if (node->n_args != 2) {
        fprintf(stderr, "Expected 2 arguments for XOR, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    int vx = is_register(arg1->value); // Returns register index or -1
    int vy = is_register(arg2->value); // Returns register index or -1

    if (vx == -1 || vy == -1) {
        fprintf(stderr, "%s argument of XOR must be a register, got %s\n", (vx == -1 ? "First" : "Second"), arg1->value);
        return;
    }

    uint16_t opcode = 0x8003 | rf(vx, 8) | rf(vy, 4);
    add_opcode(opcode);
}

void evaluate_RND(Node *node) {
    if (node->n_args != 2) {
        fprintf(stderr, "Expected 2 arguments for RND, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];
    Node *arg2 = node->args[1];

    int vx = is_register(arg1->value); // Returns register index or -1

    if (vx == -1) {
        fprintf(stderr, "First argument of RND must be a register, got %s\n", arg1->value);
        return;
    }

    if (arg2->type != NODE_NUMBER) {
        fprintf(stderr, "Second argument of RND must be a number, got type %d\n", arg2->type);
        return;
    }

    uint16_t n = nn(arg2->value);
    if (n > 0xFF) {
        fprintf(stderr, "Second argument of RND out of range (0-255), got %u\n", n);
        n = 0xFF;
    }

    uint16_t opcode = 0xC000 | rf(vx, 8) | n;
    add_opcode(opcode);
}

void evaluate_SHR(Node *node) {
    if (node->n_args != 1) {
        fprintf(stderr, "Expected 1 argument for SHR, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];

    int vx = is_register(arg1->value); // Returns register index or -1

    if (vx == -1) {
        fprintf(stderr, "Argument of SHR must be a register, got %s\n", arg1->value);
        return;
    }

    uint16_t opcode = 0x8006 | rf(vx, 8);
    add_opcode(opcode);
}

void evaluate_SHL(Node *node) {
    if (node->n_args != 1) {
        fprintf(stderr, "Expected 1 argument for SHL, got %d\n", node->n_args);
        return;
    }
    Node *arg1 = node->args[0];

    int vx = is_register(arg1->value); // Returns register index or -1

    if (vx == -1) {
        fprintf(stderr, "Argument of SHL must be a register, got %s\n", arg1->value);
        return;
    }

    uint16_t opcode = 0x800E | rf(vx, 8);
    add_opcode(opcode);
}

void evaluate_label(Node *node) {
    char *name = node->value;
    add_label(name, addr_ptr);
    pending_resolve(name, addr_ptr);
}

void evaluate_node(Node *node) {
    // print node info
    if (node->type == NODE_INSTRUCTION) {
        if (strcmp(node->value, "LD") == 0) {
            evaluate_LD(node);
        } else if (strcmp(node->value, "JP") == 0) {
            evaluate_JP(node);
        } else if (strcmp(node->value, "ADD") == 0) {
            evaluate_ADD(node);
        } else if (strcmp(node->value, "SUB") == 0) {
            evaluate_SUB(node);
        } else if (strcmp(node->value, "SUBN") == 0) {
            evaluate_SUBN(node);
        } else if (strcmp(node->value, "SE") == 0) {
            evaluate_SE(node);
        } else if (strcmp(node->value, "SNE") == 0) {
            evaluate_SNE(node);
        } else if (strcmp(node->value, "CALL") == 0) {
            evaluate_CALL(node);
        } else if (strcmp(node->value, "RET") == 0) {
            evaluate_RET(node);
        } else if (strcmp(node->value, "DB") == 0) {
            evaluate_DB(node);
        } else if (strcmp(node->value, "DRW") == 0) {
            evaluate_DRW(node);
        } else if (strcmp(node->value, "CLS") == 0) {
            evaluate_CLS(node);
        } else if (strcmp(node->value, "SKP") == 0) {
            evaluate_SKP(node);
        } else if (strcmp(node->value, "SKNP") == 0) {
            evaluate_SKNP(node);
        } else if (strcmp(node->value, "XOR") == 0) {
            evaluate_XOR(node);
        } else if (strcmp(node->value, "RND") == 0) {
            evaluate_RND(node);
        } else if (strcmp(node->value, "SHR") == 0) {
            evaluate_SHR(node);
        } else if (strcmp(node->value, "SHL") == 0) {
            evaluate_SHL(node);
        }
        else {
            fprintf(stderr, "Unknown instruction: %s\n", node->value);
        }
    } else if (node->type == NODE_NUMBER) {
        //printf("Number: %s\n", node->value);
    } else if (node->type == NODE_IDENTIFIER) {
        //printf("Identifier: %s\n", node->value);
    } else if (node->type == NODE_LABEL) {
        //printf("Label: %s at address 0x%03X\n", node->value, addr_ptr);
        //add_label(node->value, addr_ptr);
        evaluate_label(node);
    }
    else {
        printf("Unknown node type: %d\n", node->type);
    }
}

Codegen codegen_ast(Node *root) {
    label_registry = symtab_create(0);
    codegen.opcode_count = 0;
    pending_count = 0;
    pending_capacity = 0;
    pending_refs = NULL;
    memset(temp_memory, 0, sizeof(temp_memory));

    for (int i = 0; i < root->n_args; i++) {
        evaluate_node(root->args[i]);
    }
   
    if (pending_count > 0) {
        for (size_t i = 0; i < pending_count; i++) {
            fprintf(stderr, "Undefined label: %s (referenced at opcode index %u)\n",
                    pending_refs[i].label, (unsigned)pending_refs[i].opcode_index);
        }
    }

    memset(final_opcodes, 0, sizeof(final_opcodes));
    for (size_t i = 0; i < codegen.opcode_count; i++) {
        final_opcodes[i] = opcodes[i].opcode;
    }
    codegen.opcodes = final_opcodes;
    memcpy(codegen.hard_values, temp_memory, sizeof(temp_memory));
    //codegen.opcodes = opcodes;
    
    symtab_destroy(label_registry);
    label_registry = NULL;
    pending_clear_all();
    return codegen;
}