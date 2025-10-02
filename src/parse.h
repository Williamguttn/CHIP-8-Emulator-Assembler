#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include "lexer.h"
#include "token.h"

typedef enum {
    NODE_NUMBER,
    NODE_PROGRAM,
    NODE_INSTRUCTION,
    NODE_LABEL,
    NODE_DEREFERENCE,
    NODE_IDENTIFIER
} NodeType;

typedef struct Node {
    NodeType type;
    char *value;
    struct Node *left;
    struct Node *right;
    
    struct Node **args; // For instructions with multiple arguments
    int n_args;
} Node;

typedef struct {
    Lexer *lexer;
    Token *curr_token;
} Parser;

Parser *init_parser(Lexer *lexer);
Node *parse(Parser *parser);

#endif // PARSE_H