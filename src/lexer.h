#ifndef LEXER_H
#define LEXER_H

typedef struct {
    char *source;
    int position;
    int line;
    char current_char;
} Lexer;

Lexer *init_lexer(const char *source);

void advance(Lexer *lexer);
void skip_whitespace(Lexer *lexer);
char *read_number(Lexer *lexer);
char *read_identifier(Lexer *lexer);

#endif // LEXER_H