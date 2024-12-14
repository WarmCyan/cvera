/* ====================================================================
Original Copyright (c) 2024 Devine Lu Linvega
Modified Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

static char delim; /* the spacer glyph delimiter, conventionally '|' */

/* iterate the pointer until we find the first non-whitespace character */ 
static char* walk_whitespace(char* s) {
    while (s[0] && s[0] <= 0x20) /* 'space' and below */
        s++;
    return s;
}

/* find the next null terminator */
static char* walk_until_null(char* s) {
    while (*s) s++;
    return s;
}

/* turn all whitespace characters at end of string to null terms */
static void trim(char** s) {
    char* end = walk_until_null(*s) - 1;
    /* work backwards from the end until we find a real letter. */
    while (*end) {
        if (*end > 0x20) break; /* I'm a real letter! */
        *end = 0; /* make this a null term */
        end--; /* work backwards */
    }
}

/* check if two passed symbols are the same */
static int compare_symbols(char* a, char* b) {
    while (*a && *b) {
        /* stop when we've reached the end of one of the symbols */
        if (*a == ',' || *a == delim || *b == ',' || *b == delim) break;
        /* obviously if they don't equal, stop! */
        if (*a != *b) break;
        /* we don't care about differing amounts of whitespace within symbols,
         * so just jump ahead to nonwhitespace. Note that walk_whitespace jumps
         * to first NON-ws character, so go back one to account for increments
         * below.*/
        if (*a == ' ') a = walk_whitespace(a) - 1;
        if (*b == ' ') b = walk_whitespace(b) - 1;
        /* go to next character in each symbol */
        a++;
        b++;
    }
    /* symbols are the same if we successfully made it to the end of both
     * symbols - this can either mean the symbol is at a null term, or at a
     * comma or delimiter */
    /* TODO: this may not account for untrimmed symbols, which I think is why
     * original had an additional *a <= 0x20 check */
    return (!*a || *a == ',' || *a == delim) && (!*b || *b == ',' || *b == delim);
}
