#ifndef LEXER_H
#define LEXER_H

// lexer.h - Header for simple lexer library.
// By K. Walsh <kwalsh@cs.holycross.edu>, Spring 2013
//
// Based closely on:
// Parser class header for MACS442, Spring 2002
// Colorado School of Mines
// by Nick Bauer

// The lexer takes a string and breaks it up into tokens. Each token is a
// a plain word, or a double-quoted or single-quoted string. Escapes
// are handled within quoted strings, and whitespace can be escaped outside of
// quoted strings. Anything following a '#' character is ignored.
// Special

enum TokenType {
    NONE = 0,
    WORD = 1,
    // special[0] = 2,
    // special[1] = 3,
    // ...
    // special[i] = 2+i,
    // ...
};

// Lexer state
struct lexer {
    char *line;          // a private copy of the string to be lexed
    int len;             // total length of line, not including null terminator
    int pos;             // number of chars processed so far
    const char *errmsg;  // most recent error message, or NULL if no errors so far
    char *tstr;          // the current token as a string
    TokenType ttype;     // the type of the current token
    char **specials;     // NULL terminated list of special tokens (TokenType = 2+i)
};

// Initialize the lexer with a new string to be lexed.
void lexer_init(lexer *x, const char *line);

// Advance to next token. 
void lexer_next(lexer *x);

// De-initialize the lexer, freeing up memory allocated during lexer_init().
void lexer_destroy(lexer *x);

#endif // LEXER_H
