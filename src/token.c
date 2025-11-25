#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *INSTRUCTIONS[] = {
    "LD", "ADD", "JP", "SUB", "SE", "SNE", "CALL", "RET", "DB", "DRW", "CLS",
    "SKNP", "SKP", "XOR", "RND",
    /* Undocumented */
    "SUBN", "SHR", "SHL",
    NULL
};

char *is_instruction(const char *value) {
    for (size_t i = 0; INSTRUCTIONS[i] != NULL; i++) {
        if (strcmp(value, INSTRUCTIONS[i]) == 0) {
            return INSTRUCTIONS[i];
        }
    }

    return NULL;
}

Token *get_next_token(Lexer *lexer) {
    skip_whitespace(lexer);
    Token *token = (Token *)malloc(sizeof(Token));

    if (lexer->current_char == '\0') {
        token->type = TOKEN_EOF;
        token->value = NULL;
        return token;
    } else if (lexer->current_char == '\n') {
        token->type = TOKEN_EOL;
        token->value = NULL;
        advance(lexer);
        return token;
    } else if (lexer->current_char == ',') {
        token->type = TOKEN_COMMA;
        token->value = ",";
        advance(lexer);
        return token;
    } else if (lexer->current_char == ':') {
        token->type = TOKEN_COLON;
        token->value = ":";
        advance(lexer);
        return token;
    } else if (lexer->current_char == '[') {
        token->type = TOKEN_SQ_BRACKET_OPEN;
        token->value = "[";
        advance(lexer);
        return token;
    } else if (lexer->current_char == ']') {
        token->type = TOKEN_SQ_BRACKET_CLOSE;
        token->value = "]";
        advance(lexer);
        return token;
    }
    else if (isdigit((unsigned char)lexer->current_char)) {
        token->type = TOKEN_NUMBER;
        token->value = read_number(lexer);
        return token;
    } else if (isalpha((unsigned char)lexer->current_char) || lexer->current_char == '_') {
        char *ident = read_identifier(lexer);
        char *instr = is_instruction(ident);

        if (instr != NULL) {
            if (strcmp(instr, "LD") == 0) {
                token->type = TOKEN_LD;
            } else if (strcmp(instr, "ADD") == 0) {
                token->type = TOKEN_ADD;
            } else if (strcmp(instr, "SUB") == 0) {
                token->type = TOKEN_SUB;
            } else if (strcmp(instr, "SUBN") == 0) {
                token->type = TOKEN_SUBN;
            } else if (strcmp(instr, "JP") == 0) {
                token->type = TOKEN_JP;
            } else if (strcmp(instr, "SE") == 0) {
                token->type = TOKEN_SE;
            } else if (strcmp(instr, "SNE") == 0) {
                token->type = TOKEN_SNE;
            } else if (strcmp(instr, "SHL") == 0) {
                token->type = TOKEN_SHL;
            } else if (strcmp(instr, "SHR") == 0) {
                token->type = TOKEN_SHR;
            } else if (strcmp(instr, "CALL") == 0) {
                token->type = TOKEN_CALL;
            } else if (strcmp(instr, "RET") == 0) {
                token->type = TOKEN_RET;
            } else if (strcmp(instr, "DB") == 0) {
                token->type = TOKEN_DB;
            } else if (strcmp(instr, "DRW") == 0) {
                token->type = TOKEN_DRW;
            } else if (strcmp(instr, "CLS") == 0) {
                token->type = TOKEN_CLS;
            } else if (strcmp(instr, "SKNP") == 0) {
                token->type = TOKEN_SKNP;
            } else if (strcmp(instr, "SKP") == 0) {
                token->type = TOKEN_SKP;
            } else if (strcmp(instr, "XOR") == 0) {
                token->type = TOKEN_XOR;
            } else if (strcmp(instr, "RND") == 0) {
                token->type = TOKEN_RND;
            }
            else {
                token->type = TOKEN_UNKNOWN;
            }
            token->value = instr;
            free(ident);
            return token;
        }

        token->type = TOKEN_IDENTIFIER;
        token->value = ident;
        return token;
    } else {
        token->type = TOKEN_UNKNOWN;
        token->value = NULL;
        advance(lexer);
        return token;
    }
}
