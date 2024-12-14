/* ====================================================================
Original Copyright (c) 2024 Devine Lu Linvega
Modified Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

struct sym_table {
    /* pointer to an array of all unique symbol names separated by null terms */
    char* names; 
    /* pointer to an array of pointers, the latter of which is each into the sym
     * names array. */
    char** table;
    int max_len; /* bounds for the syms table */
    int len; /* current number of syms in table */
    /* the last stop within the names string array, start next new symbol here. */
    int names_len;
    /* int max_names_len */
};

static char delim; /* the spacer glyph delimiter, conventionally '|' */
/* static char symbol_strings[]; */
/* static char symbol_strings_len */
/*  */
/* static int syms_table_len = 0;  how many symbols are in the symbols table */ */

/* iterate the pointer until we find the first non-whitespace character */ 
static char* walk_whitespace(char* s) {
    while (s && s <= 0x20) /* 'space' and below */
        s++;
    return s;
}

/* turn all whitespace characters at end of string to null terms */
static void trim(char** s) {
    /* find the first null terminator so we can work backwards. */
    char* end = *s;
    while (*end) end++;
    end = end - 1;
    /* work backwards from the end until we find a real letter. */
    while (*end) {
        if (*end > 0x20) break; /* I'm a real letter! */
        *end = 0; /* make this a null term */
        end--; /* work backwards */
    }
}

/* check if two passed symbols are the same */
/* I think the original assumption is that a is always the source code? hence
 * the assymetry originally */
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

/* find ID of symbol in a symbols table */
static int index_of_symbol(char* s, sym_table* syms) {
    int i = 0;
    for (i = 0; i < *syms.len; i++) {
        if (compare_symbols(s, *syms.table[i])) {
            return i;
        }
    }
    return -1;
}

/* walk to the end of the next symbol, adding it to the symbol table if it
 * hasn't been seen before. (and store the index to that symbol) */ 
static char* walk_symbol(char* s, int* id, sym_table* syms) {
    s = walk_whitespace(s);
    *id = index_of_symbol(s, sym_table, sym_table_len);
    if (*id > -1) {
        /* we've seen this symbol before, so just walk to the end of it 
         * (when we see a delimiter or end of fact syntax) */
        while (s && s[0] != delim && s[0] != ',') *s++;
        return s;
    }
    
    /* new symbol found! Woo! */
    /* assign the next symbol in the symbols table to the current position of
     * the symbol names string. */
    *syms.table[*syms.len] = *syms.names[*syms.names_len];
    *syms.len++;
    /* write the symbol string to the names array */
    while (s && s[0] != delim && s[0] != ',') {
        *syms.names[*syms.names_len] = s[0];
        *syms.names_len++;
        /* skip anything more than one whitespace. TODO: should eventually be
         * sep pass */
        if (*s == ' ') {
            s = walk_whitespace(s);
        }
        s++;
    }
    /* trim any whitespace off the end TODO: this should eventually be sep 
     * pass */
    trim(*syms.table[*syms.len - 1]);
    *id = *syms.len - 1;
    return s;
}
