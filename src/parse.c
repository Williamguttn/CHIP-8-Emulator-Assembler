#include "parse.h"
#include <stdlib.h>
#include <string.h>

Parser *init_parser(Lexer *lexer) {
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->curr_token = get_next_token(lexer);
    return parser;
}

Node *create_node(NodeType type, char *value) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->left = NULL;
    node->right = NULL;
    node->args = NULL;
    node->n_args = 0;
    return node;
}

void *eat(Parser *parser, TokenType type) {
    if (parser->curr_token->type == type) {
        Token *new_token = get_next_token(parser->lexer);
        parser->curr_token = new_token;
        return NULL;
    } else {
        fprintf(stderr, "Unexpected token: expected %d, got %d\n", type, parser->curr_token->type);
        return (void *)1; // Indicate error
    }
}

// Own function since it expects 1 arg
Node *parse_jp(Parser *parser) {

    Node *node = create_node(NODE_INSTRUCTION, "JP");
    eat(parser, TOKEN_JP);
    
    // arg node can either be a number or identifier
    if (parser->curr_token->type == TOKEN_NUMBER) {
        Node *arg = create_node(NODE_NUMBER, parser->curr_token->value);
        node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
        node->args[node->n_args++] = arg;
        eat(parser, TOKEN_NUMBER);
    } else if (parser->curr_token->type == TOKEN_IDENTIFIER) {
        Node *arg = create_node(NODE_IDENTIFIER, parser->curr_token->value);
        node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
        node->args[node->n_args++] = arg;
        eat(parser, TOKEN_IDENTIFIER);
    } else {
        fprintf(stderr, "JP instruction expects a number or identifier as argument, got token type %d\n", parser->curr_token->type);
        free(node);
        return NULL;
    }

    return node;
}

Node *parse_call(Parser *parser) {
    Node *node = create_node(NODE_INSTRUCTION, "CALL");
    eat(parser, TOKEN_CALL);
    
    if (parser->curr_token->type == TOKEN_IDENTIFIER) {
        Node *arg = create_node(NODE_IDENTIFIER, parser->curr_token->value);
        node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
        node->args[node->n_args++] = arg;
        eat(parser, TOKEN_IDENTIFIER);
    } else {
        fprintf(stderr, "CALL instruction expects an identifier as argument, got token type %d\n", parser->curr_token->type);
        free(node);
        return NULL;
    }

    return node;
}

Node *parse_ret(Parser *parser) {
    Node *node = create_node(NODE_INSTRUCTION, "RET");
    eat(parser, TOKEN_RET);
    return node; // expect no args
}

Node *parse_db(Parser *parser) {
    Node *node = create_node(NODE_INSTRUCTION, "DB");
    eat(parser, TOKEN_DB);
    
    // allow multiple number args separated by commas
    while (1) {
        if (parser->curr_token->type == TOKEN_NUMBER) {
            Node *arg = create_node(NODE_NUMBER, parser->curr_token->value);
            node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
            node->args[node->n_args++] = arg;
            eat(parser, TOKEN_NUMBER);
        } else {
            fprintf(stderr, "DB instruction expects a number as argument, got token type %d\n", parser->curr_token->type);
            free(node);
            return NULL;
        }

        if (parser->curr_token->type == TOKEN_COMMA) {
            eat(parser, TOKEN_COMMA);
            continue;
        } else {
            break;
        }
    }

    return node;
}

Node *parse_drw(Parser *parser) {
    Node *node = create_node(NODE_INSTRUCTION, "DRW");
    eat(parser, TOKEN_DRW);

    // expect 3 args: Vx, Vy, N
    int cnt = 0;

    while (cnt < 3 && parser->curr_token->type != TOKEN_EOF && parser->curr_token->type != TOKEN_EOL) {
        if (parser->curr_token->type == TOKEN_COMMA) {
            eat(parser, TOKEN_COMMA);
            continue;
        }

        if (parser->curr_token->type == TOKEN_NUMBER) {
            Node *arg_node = create_node(NODE_NUMBER, parser->curr_token->value);
            node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
            node->args[node->n_args++] = arg_node;
            eat(parser, TOKEN_NUMBER);
            cnt++;
        } else if (parser->curr_token->type == TOKEN_IDENTIFIER) {
            Node *arg_node = create_node(NODE_IDENTIFIER, parser->curr_token->value);
            node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
            node->args[node->n_args++] = arg_node;
            eat(parser, TOKEN_IDENTIFIER);
            cnt++;
        } else {
            fprintf(stderr, "DRW instruction expects a number or identifier as argument, got token type %d\n", parser->curr_token->type);
            free(node);
            return NULL;
        }
    }

    return node;
}

Node *parse_sknp(Parser *parser) {
    Node *node = create_node(NODE_INSTRUCTION, "SKNP");
    eat(parser, TOKEN_SKNP);
    
    // expect 1 arg: Vx
    if (parser->curr_token->type == TOKEN_IDENTIFIER) {
        Node *arg = create_node(NODE_IDENTIFIER, parser->curr_token->value);
        node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
        node->args[node->n_args++] = arg;
        eat(parser, TOKEN_IDENTIFIER);
    } else {
        fprintf(stderr, "SKNP instruction expects an identifier as argument, got token type %d\n", parser->curr_token->type);
        free(node);
        return NULL;
    }

    return node;
}

Node *parse_skp(Parser *parser) {
    Node *node = create_node(NODE_INSTRUCTION, "SKP");
    eat(parser, TOKEN_SKP);
    
    // expect 1 arg: Vx
    if (parser->curr_token->type == TOKEN_IDENTIFIER) {
        Node *arg = create_node(NODE_IDENTIFIER, parser->curr_token->value);
        node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
        node->args[node->n_args++] = arg;
        eat(parser, TOKEN_IDENTIFIER);
    } else {
        fprintf(stderr, "SKP instruction expects an identifier as argument, got token type %d\n", parser->curr_token->type);
        free(node);
        return NULL;
    }

    return node;
}

Node *parse_shl(Parser *parser) {
    Node *node = create_node(NODE_INSTRUCTION, "SHL");
    eat(parser, TOKEN_SHL);

    // expect 1 arg: Vx
    if (parser->curr_token->type == TOKEN_IDENTIFIER) {
        Node *arg = create_node(NODE_IDENTIFIER, parser->curr_token->value);
        node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
        node->args[node->n_args++] = arg;
        eat(parser, TOKEN_IDENTIFIER);
    } else {
        fprintf(stderr, "SHL instruction expects an identifier as argument, got token type %d\n", parser->curr_token->type);
        free(node);
        return NULL;
    }

    return node;
}

Node *parse_shr(Parser *parser) {
    Node *node = create_node(NODE_INSTRUCTION, "SHR");
    eat(parser, TOKEN_SHR);

    // expect 1 arg: Vx
    if (parser->curr_token->type == TOKEN_IDENTIFIER) {
        Node *arg = create_node(NODE_IDENTIFIER, parser->curr_token->value);
        node->args = (Node **)realloc(node->args, sizeof(Node *) * (node->n_args + 1));
        node->args[node->n_args++] = arg;
        eat(parser, TOKEN_IDENTIFIER);
    } else {
        fprintf(stderr, "SHR instruction expects an identifier as argument, got token type %d\n", parser->curr_token->type);
        free(node);
        return NULL;
    }

    return node;
}

Node *parse_instruction(Parser *parser) {
    Node *node = create_node(NODE_INSTRUCTION, NULL);

    if (parser->curr_token->type == TOKEN_LD) {
        node->value = "LD";
        eat(parser, TOKEN_LD);
    } else if (parser->curr_token->type == TOKEN_ADD) {
        node->value = "ADD";
        eat(parser, TOKEN_ADD);
    } else if (parser->curr_token->type == TOKEN_SUB) {
        node->value = "SUB";
        eat(parser, TOKEN_SUB);
    } else if (parser->curr_token->type == TOKEN_SUBN) {
        node->value = "SUBN";
        eat(parser, TOKEN_SUBN);
    } else if (parser->curr_token->type == TOKEN_SE) {
        node->value = "SE";
        eat(parser, TOKEN_SE);
    } else if (parser->curr_token->type == TOKEN_SNE) {
        node->value = "SNE";
        eat(parser, TOKEN_SNE);
    } else if (parser->curr_token->type == TOKEN_XOR) {
        node->value = "XOR";
        eat(parser, TOKEN_XOR);
    } else if (parser->curr_token->type == TOKEN_RND) {
        node->value = "RND";
        eat(parser, TOKEN_RND);
    } else if (parser->curr_token->type == TOKEN_SHL) {
        return parse_shl(parser);
    } else if (parser->curr_token->type == TOKEN_SHR) {
        return parse_shr(parser);
    } else if (parser->curr_token->type == TOKEN_CALL) {
        return parse_call(parser);
    } else if (parser->curr_token->type == TOKEN_RET) {
        return parse_ret(parser);
    } else if (parser->curr_token->type == TOKEN_DB) {
        return parse_db(parser);
    } else if (parser->curr_token->type == TOKEN_JP) {
        /*node->value = "JP";
        eat(parser, TOKEN_JP);*/
        return parse_jp(parser);
    } else if (parser->curr_token->type == TOKEN_DRW) {
        return parse_drw(parser);
    } else if (parser->curr_token->type == TOKEN_CLS) {
        node->value = "CLS";
        eat(parser, TOKEN_CLS);
        return node; // expect no args
    } else if (parser->curr_token->type == TOKEN_SKNP) {
        return parse_sknp(parser);
    } else if (parser->curr_token->type == TOKEN_SKP) {
        return parse_skp(parser);
    }
    else {
        fprintf(stderr, "Expected instruction, got token type %d\n", parser->curr_token->type);
        free(node);
        return NULL;
    }

    // Parse arguments
    Node **args = NULL;
    int arg_count = 0;

    while (parser->curr_token->type != TOKEN_EOL && parser->curr_token->type != TOKEN_EOF) {
        if (parser->curr_token->type == TOKEN_COMMA) {
            eat(parser, TOKEN_COMMA);
            continue;
        }

        if (parser->curr_token->type == TOKEN_NUMBER) {
            Node *arg_node = create_node(NODE_NUMBER, parser->curr_token->value);
            args = (Node **)realloc(args, sizeof(Node *) * (arg_count + 1));
            args[arg_count++] = arg_node;
            eat(parser, TOKEN_NUMBER);
        } else if (parser->curr_token->type == TOKEN_IDENTIFIER) {
            Node *arg_node = create_node(NODE_IDENTIFIER, parser->curr_token->value);
            args = (Node **)realloc(args, sizeof(Node *) * (arg_count + 1));
            args[arg_count++] = arg_node;
            eat(parser, TOKEN_IDENTIFIER);
        } else if (parser->curr_token->type == TOKEN_SQ_BRACKET_OPEN) {
            // Handle dereference
            eat(parser, TOKEN_SQ_BRACKET_OPEN);
            if (parser->curr_token->type != TOKEN_IDENTIFIER) {
                fprintf(stderr, "Expected identifier after '[', got token type %d\n", parser->curr_token->type);
                // Free args and return NULL
                for (int i = 0; i < arg_count; i++) {
                    free(args[i]->value);
                    free(args[i]);
                }
                free(args);
                free(node);
                return NULL;
            }
            Node *arg_node = create_node(NODE_DEREFERENCE, parser->curr_token->value);
            args = (Node **)realloc(args, sizeof(Node *) * (arg_count + 1));
            args[arg_count++] = arg_node;
            eat(parser, TOKEN_IDENTIFIER);

            if (parser->curr_token->type != TOKEN_SQ_BRACKET_CLOSE) {
                fprintf(stderr, "Expected ']' after identifier, got token type %d\n", parser->curr_token->type);
                // Free args and return NULL
                for (int i = 0; i < arg_count; i++) {
                    free(args[i]->value);
                    free(args[i]);
                }
                free(args);
                free(node);
                return NULL;
            }
            eat(parser, TOKEN_SQ_BRACKET_CLOSE);
        }
        else {
            fprintf(stderr, "Unexpected token in arguments: type %d of value %s\n", parser->curr_token->type, parser->curr_token->value);
            // Free args and return NULL
            for (int i = 0; i < arg_count; i++) {
                free(args[i]->value);
                free(args[i]);
            }
            free(args);
            free(node);
            return NULL;
        }
    }
    node->args = args;
    node->n_args = arg_count;
    return node;
}

Node *parse_label(Parser *parser) {
    if (parser->curr_token->type != TOKEN_IDENTIFIER) {
        return parse_instruction(parser);
    }

    char *label_name = strdup(parser->curr_token->value);
    eat(parser, TOKEN_IDENTIFIER);

    if (parser->curr_token->type != TOKEN_COLON) {
        fprintf(stderr, "Expected colon after label name, got token type %d\n", parser->curr_token->type);
        free(label_name);
        return NULL;
    }
    eat(parser, TOKEN_COLON);

    Node *label_node = create_node(NODE_LABEL, label_name);
    free(label_name);
    return label_node;
}

Node *parse(Parser *parser) {
    Node *program = create_node(NODE_PROGRAM, NULL);

    while (parser->curr_token->type != TOKEN_EOF) {
        if (parser->curr_token->type == TOKEN_EOL) {
            eat(parser, TOKEN_EOL);
            continue;
        }
        
        Node *instr_node = parse_label(parser);

        if (!instr_node) {
            fprintf(stderr, "Failed to parse instruction.\n");
            free(program);
            return NULL;
        }

        program->args = (Node **)realloc(program->args, sizeof(Node *) * (program->n_args + 1));
        program->args[program->n_args++] = instr_node;

        if (parser->curr_token->type != TOKEN_EOL && parser->curr_token->type != TOKEN_EOF) {
            fprintf(stderr, "Expected EOL or EOF after instruction, got %d\n", parser->curr_token->type);
            // Handle error
            break;
        }
    }

    return program;
}