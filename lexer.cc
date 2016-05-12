// lexer.cc - Simple lexer library in plain C.
// See lexer.h for documentation regarding the use of these functions.
// By K. Walsh <kwalsh@cs.holycross.edu>, Spring 2016
//
// Based very loosely on:
// Parser class implementation for MACS442, Spring 2002
// Colorado School of Mines
// by Nick Bauer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "stringlist.h"


void lexer_init(lexer *x, const char *line)
{
    x->line = strdup(line);
    x->len = strlen(line);
    x->pos = 0;
    x->errmsg = NULL;
    x->tstr = NULL;
    x->ttype = NONE;
    x->specials = stringlist_empty();
}

void lexer_destroy(lexer *x)
{
    free(x->line);
    x->len = x->pos = 0;
    x->errmsg = NULL;
    if (x->tstr) {
        free(x->tstr);
        x->tstr = NULL;
    }
    x->ttype = NONE;
}

// Get the character a the current position, or '\0' if at end of line.
static char lexer_ch(lexer *x)
{
    if (x->pos >= x->len)
        return '\0';
    else
        return x->line[x->pos];
}

// Get the suffix starting at the current position, or NULL if at end of line.
static char *lexer_str(lexer *x)
{
    if (x->pos >= x->len)
        return NULL;
    else
        return &x->line[x->pos];
}

// Check if c is whitespace.
static bool is_blank(char c)
{
    return (c == '\0' || c == ' ' || c == '\t');
}

// Check if s begins with an escaped whitespace, comment character, or quote.
static bool is_escaped_whitespace(lexer *x, char *s)
{
    if (s != NULL && s[0] == '\\') {
        switch (s[1]) {
            case ' ':
            case '\t':
            case '\\':
            case '#':
            case '\'':
            case '"':
                return true;
            case '\0':
                x->errmsg = "Error parsing string: trailing backslash is not allowed.";
                break;
            default:
                x->errmsg = "Error parsing string: invalid escape outside of quoted string.";
                break;
        }
    }
    return false;
}

// Check if c is the start of a special symbol. Returns the index if matched,
// otherwise returns -1.
static int match_special(lexer *x, char *s)
{
    if (s == NULL)
        return -1;
    for (int i = 0; x->specials[i] != NULL; i++) {
        int n = strlen(x->specials[i]);
        if (!strncmp(s, x->specials[i], n)) {
            return i;
        }
    }
    return -1;
}

// Overwrite a string with an un-escaped version of itself. Returns NULL on success,
// error message otherwise.
// "\n" is replaced by '\n'
// "\r" is replaced by '\r'
// "\t" is replaced by '\t'
// "\\" is replaced by '\\'
// "\'" is replaced by '\''
// "\"" is replaced by '\"'
// "\x" is replaced by 'x' (where x is any other character not listed above)
static void lexer_unescape(lexer *x, char *original)
{
    char *fixed = original;
    while (original && *original) {
        char c = *original++;
        if (c == '\\') {
            // If we just saw a backslash, look at the next character
            c = *original++;
            switch(c) {
                case '\0':
                    x->errmsg = "Error parsing string: line can't end in '\\'.";
                    return;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case '\'':
                    case '\"':
                case '\\':
                default:
                        // leave c alone
                        break;
            }
        }
        *fixed++ = c;
    }
    *fixed = '\0'; // null terminate the fixed string
}

// set the current token to WORD str, unescaped
static void lexer_set(lexer *x, char *str) {
    // Overwrite the string with an un-escaped version of itself.
    lexer_unescape(x, str);
    if (x->errmsg) {
        free(str);
        return;
    } else {
        x->tstr = str;
        x->ttype = WORD;
    }
}

// Advance to next token. 
void lexer_next(lexer *x)
{
    if (x->tstr) {
        free(x->tstr);
        x->tstr = NULL;
    }
    x->ttype = NONE;

    // Skip whitespace before the token
    while (lexer_ch(x) && is_blank(lexer_ch(x)))
        x->pos++;

    // No characters left in line?
    if (!lexer_ch(x))
        return;

    // Examine the suffix starting at the current position
    char *s = lexer_str(x);
    int startpos = x->pos++;

    int i;

    if (s[0] == '#') {
        x->pos = x->len; // skip to end
    } else if ((i = match_special(x, s)) >= 0) {
        int n = strlen(x->specials[i]);
        x->pos += (n-1);
        x->tstr = strndup(s, n);
        x->ttype = (TokenType)(2 + i);
    } else if (s[0] == '\'' || s[0] == '"') {
        // This is a quoted string, so find the matching quote
        while (lexer_ch(x) && lexer_ch(x) != s[0]) {
            if (lexer_ch(x) == '\\') {
                // If we see a backslash, skip this character...
                x->pos++;
                // ... and the next (if there is one)
                if (lexer_ch(x))
                    x->pos++;
            } else {
                // If it wasn't a backslash, skip it.
                x->pos++;
            }
        }
        if (!lexer_ch(x)) {
            x->errmsg = "Error parsing string: no matching quote.";
            return;
        }
        // Copy the inside of the quoted string
        int endpos = x->pos;
        x->pos++;
        char *str = strndup(s+1, endpos-startpos-1);
        lexer_set(x, str);
    } else {
        // Unquoted word, just look for next word boundary or end of line.
        // But also skip over escaped whitespace.
        if (is_escaped_whitespace(x, s)) {
            x->pos++;
        }
        for (;;) {
            if (is_blank(lexer_ch(x)))
                break;
            if (lexer_ch(x) == '#')
                break;
            if (match_special(x, lexer_str(x)) != -1)
                break;
            if (is_escaped_whitespace(x, lexer_str(x)))
                x->pos++;
            x->pos++;
        }
        int endpos = x->pos;
        char *str = strndup(s, endpos-startpos);
        lexer_set(x, str);
    }
}


