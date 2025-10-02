#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emulate.h"
#include "interpreter.h"
#include "lexer.h"
#include "parse.h"
#include "codegen.h"
#include "graphics.h"
#include "opts.h"
#include "preprocess.h"

#define RUN_CODEGEN 1

int main(int argc, char *argv[]) {

    Options opts = parse_args(argc, argv);

    if (opts.input_mode == INPUT_NONE) {
        printf("No input mode specified. Use --help for help.\n");
    }

    if (opts.input_mode == INPUT_ASM) {
        char *source = load_file_asm(opts.input_file);
        if (!source) {
            return 1;
        }

        if (opts.run || opts.output_rom || opts.output_hex) {
            Lexer *lexer = init_lexer(source);
            if (!lexer) {
                free(source);
                return 1;
            }

            Parser *parser = init_parser(lexer);
            if (!parser) {
                free(lexer->source);
                free(lexer);
                free(source);
                return 1;
            }

            Node *ast = parse(parser);
            if (!ast) {
                fprintf(stderr, "Parsing failed.\n");
                free(lexer->source);
                free(lexer);
                free(source);
                return 1;
            }

            Codegen codegen = codegen_ast(ast);
            Opcodes opcodes = interpret_codegen(codegen);
            uint8_t memory[4096];
            int write_output_rom = (opts.output_rom != NULL) ? 1 : 0;
            int write_output_hex = (opts.output_hex != NULL) ? 1 : 0;

            int skip_emulation = (write_output_rom && !opts.run) || (write_output_hex && !opts.run);

            init_emulator(opcodes, memory, skip_emulation);

            if (write_output_rom) {
                int ret = write_rom(opts.output_rom, memory, sizeof(memory));
                if (ret != 0) {
                    fprintf(stderr, "Error writing ROM file: %s\n", strerror(errno));
                    free_opcodes(&opcodes);
                    free(ast);
                    free(parser);
                    free(lexer->source);
                    free(lexer);
                    free(source);
                    return 1;
                }
            } else if (write_output_hex) {
                write_hex(opts.output_hex, memory, sizeof(memory));
            }

            free_opcodes(&opcodes);
            free(ast);
            free(parser);
            free(lexer->source);
            free(lexer);
            free(source);
        }
    } else if (opts.input_mode == INPUT_ROM) {
        uint8_t memory[4096];
        read_rom(opts.input_file, memory, sizeof(memory));

        // If no code is at 0x200, then move it there
        relocate_rom(memory);
        init_emulator_rom(memory);
    } else if (opts.input_mode == INPUT_HEX) {
        Opcodes opcodes = interpret_file_text(opts.input_file);
        uint8_t memory[4096];
        int write_output = (opts.output_rom != NULL) ? 1 : 0;

        init_emulator(opcodes, memory, write_output && !opts.run);

        if (write_output) {
            int ret = write_rom(opts.output_rom, memory, sizeof(memory));
            if (ret != 0) {
                fprintf(stderr, "Error writing ROM file: %s\n", strerror(errno));
                free_opcodes(&opcodes);
                return 1;
            }
        }
        free_opcodes(&opcodes);
    }

    return 0;
}