#ifndef TOKEN_H
#define TOKEN_H

#include "lexer.h"

extern char *INSTRUCTIONS[];

typedef enum {
    TOKEN_COMMA,
    TOKEN_EOL,
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_COLON,
    TOKEN_IDENTIFIER,
    TOKEN_SQ_BRACKET_OPEN,
    TOKEN_SQ_BRACKET_CLOSE,

    TOKEN_LD,
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_JP,
    TOKEN_CALL,
    TOKEN_RET,
    TOKEN_DB,
    TOKEN_DRW,
    TOKEN_CLS,
    
    // Not originally documented
    TOKEN_XOR,
    TOKEN_SUBN,
    TOKEN_SHR,
    TOKEN_SHL,
    
    TOKEN_RND,

    TOKEN_SKNP, // Skip if key not pressed
    TOKEN_SKP, // Skip if key pressed

    TOKEN_SE, // Skip if equal
    TOKEN_SNE, // Skip if not equal

    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

Token *get_next_token(Lexer *lexer);

#endif // TOKEN_H