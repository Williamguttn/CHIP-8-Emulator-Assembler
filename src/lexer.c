#include "lexer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

Lexer *init_lexer(const char *source) {
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    lexer->source = strdup(source);

    if (!lexer->source) {
        fprintf(stderr, "Failed to allocate memory for lexer source.\n");
        free(lexer);
        return NULL;
    }

    lexer->position = 0;
    lexer->line = 1;
    lexer->current_char = lexer->source[lexer->position];
    return lexer;
}

void advance(Lexer *lexer) {
    lexer->position++;
    if ((size_t)lexer->position < strlen(lexer->source)) {
        lexer->current_char = lexer->source[lexer->position];
    } else {
        lexer->current_char = '\0';
    }
}

void skip_whitespace(Lexer *lexer) {
    while (lexer->current_char == ' ' || lexer->current_char == '\t') {
        advance(lexer);
    }
}

// TODO: free
char *read_number(Lexer *lexer) {
    int start = lexer->position;

    if (lexer->current_char == '0' &&
        (lexer->source[lexer->position + 1] == 'x' || lexer->source[lexer->position + 1] == 'X')) {

        // If there is at least one hex digit after 0x/0X, consume hex form.
        if (isxdigit((unsigned char)lexer->source[lexer->position + 2])) {
            // consume '0' and 'x'/'X'
            advance(lexer);
            advance(lexer);

            while (isxdigit((unsigned char)lexer->current_char)) {
                advance(lexer);
            }

            int length = lexer->position - start;
            char *number_str = (char *)malloc(length + 1);
            if (!number_str) {
                fprintf(stderr, "Failed to allocate memory for number string.\n");
                return NULL;
            }
            strncpy(number_str, lexer->source + start, length);
            number_str[length] = '\0';
            return number_str;
        } else {
            // Not a valid hex literal (no hex digits after 0x); treat as single '0'
            advance(lexer);
            int length = lexer->position - start;
            char *number_str = (char *)malloc(length + 1);
            if (!number_str) {
                fprintf(stderr, "Failed to allocate memory for number string.\n");
                return NULL;
            }
            strncpy(number_str, lexer->source + start, length);
            number_str[length] = '\0';
            return number_str;
        }
    }

    // Decimal number path
    while (isdigit((unsigned char)lexer->current_char)) {
        advance(lexer);
    }

    int length = lexer->position - start;
    char *number_str = (char *)malloc(length + 1);
    if (!number_str) {
        fprintf(stderr, "Failed to allocate memory for number string.\n");
        return NULL;
    }
    strncpy(number_str, lexer->source + start, length);
    number_str[length] = '\0';
    return number_str;
}


char *read_identifier(Lexer *lexer) {
    int start = lexer->position;

    while (isalnum(lexer->current_char) || lexer->current_char == '_') {
        advance(lexer);
    }

    int length = lexer->position - start;
    char *id_str = (char *)malloc(length + 1);
    if (!id_str) {
        fprintf(stderr, "Failed to allocate memory for identifier string.\n");
        return NULL;
    }
    strncpy(id_str, lexer->source + start, length);
    id_str[length] = '\0';
    return id_str;
}