#include "opts.h"
#include <getopt.h>

#define OP_PER_FRAME 10

#define WINDOW_WIDTH_DEFAULT 640
#define WINDOW_HEIGHT_DEFAULT 320

Options GLOB_OPTS;

void get_window_size(char *arg, Options *opts) {
    const int MIN_WIDTH = 100;
    const int MAX_WIDTH = 7680;
    const int MIN_HEIGHT = 100;
    const int MAX_HEIGHT = 4320;

    char *input = strdup(arg);
    char *x_pos = strchr(input, 'x');
    if (x_pos == NULL) {
        fprintf(stderr, "Invalid window size format.\n");
        free(input);
        return;
    }

    *x_pos = '\0';
    char *width_str = input;
    char *height_str = x_pos + 1;

    char *endptr;
    long width = strtol(width_str, &endptr, 10);
    long height = strtol(height_str, &endptr, 10);
    if (*endptr != '\0' || width < MIN_WIDTH || width > MAX_WIDTH || height < MIN_HEIGHT || height > MAX_HEIGHT) {
        fprintf(stderr, "Invalid window size format.\n");
        free(input);
        return;
    }

    opts->window_width = (int)width;
    opts->window_height = (int)height;

    free(input);
}

void print_help(const char *prog) {
    printf("Usage: %s [options]\n", prog);
    printf("\nInput options:\n");

    printf("  --asm <file>              Assemble CHIP-8 assembly source.\n");
    printf("  --hex <file>              Load raw hex opcodes.\n");
    printf("  --rom <file>              Load CHIP-8 ROM binary.\n");

    printf("\nAction options:\n");
    printf("  -r, --run                 Run after loading.\n");
    printf("  -o, --output <file>       Output file.\n");
    printf("  -x, --hexout <file>       Write assembled hex opcodes to a text file.\n");

    printf("\nGeneral options:\n");
    printf("  -h, --help                Display this help message.\n");
    printf("  -n, --opcodes N           Number of opcodes to run per frame (default: %d).\n", OP_PER_FRAME);
    printf("  -g, --no-graphics         Disable graphics.\n");
    printf("  -p, --print-regs          Print registers after each frame.\n");
    printf("  -d, --debug               Enable debug mode.\n");
    printf("  -w, --window-size <WxH>   Set window size (default: %dx%d).\n", WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT);
}

Options parse_args(int argc, char **argv) {
    Options opts = {
        .input_mode = INPUT_NONE,
        .input_file = NULL,

        .run = 0,
        .output_rom = NULL,
        .output_hex = NULL,

        .opcodes_per_frame = OP_PER_FRAME,
        .graphics_enabled = 1,
        .print_registers = 0,
        .debug_enabled = 0,

        .window_width = WINDOW_WIDTH_DEFAULT,
        .window_height = WINDOW_HEIGHT_DEFAULT
    };

    static struct option long_opts[] = {
        {"help",        no_argument,       0, 'h'},
        {"opcodes",     required_argument, 0, 'n'},
        {"no-graphics", no_argument,       0, 'g'},
        {"print-regs",  no_argument,       0, 'p'},
        {"debug",       no_argument,       0, 'd'},
        {"window-size", required_argument, 0, 'w'},
        {"output",      required_argument, 0, 'o'},
        {"hexout",      required_argument, 0, 'x'},
        {"run",         no_argument,       0, 'r'},
        {"asm",         required_argument, 0, 1000},
        {"hex",         required_argument, 0, 1001},
        {"rom",         required_argument, 0, 1002},
        {0,0,0,0}
    };

    int opt;
    int opt_index = 0;

    while ((opt = getopt_long(argc, argv, "hn:gpdo:x:rw:", long_opts, &opt_index)) != -1) {
        switch (opt) {
            case 'h':
                print_help(argv[0]);
                exit(0);
            case 'n':
                opts.opcodes_per_frame = atoi(optarg);
                break;
            case 'g':
                opts.graphics_enabled = 0;
                break;
            case 'p':
                opts.print_registers = 1;
                break;
            case 'd':
                opts.debug_enabled = 1;
                break;
            case 'w':
                get_window_size(optarg, &opts);
                break;
            case 'o':
                opts.output_rom = strdup(optarg);
                break;
            case 'x':
                opts.output_hex = strdup(optarg);
                break;
            case 'r':
                opts.run = 1;
                break;

            // custom long-only options
            case 1000: // --asm
                opts.input_mode = INPUT_ASM;
                opts.input_file = strdup(optarg);
                break;
            case 1001: // --hex
                opts.input_mode = INPUT_HEX;
                opts.input_file = strdup(optarg);
                break;
            case 1002: // --rom
                opts.input_mode = INPUT_ROM;
                opts.input_file = strdup(optarg);
                break;

            case '?':
                fprintf(stderr, "Try --help for usage.\n");
                exit(1);
            default:
                break;
        }
    }

    GLOB_OPTS = opts;

    return opts;
}