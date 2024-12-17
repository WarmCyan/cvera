/* ====================================================================
Copyright (c) 2024 Devine Lu Linvega
Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

#include "parser.h"
#include <stdio.h>


static char delim; /* the spacer glyph delimiter, conventionally '|' */

/* iterate the pointer until we find the first non-whitespace character */ 
static char* walk_whitespace(char* s) {
    while (s[0] && s[0] <= 0x20) /* 'space' and below */
        s++;
    return s;
}

/* turn all whitespace characters at end of string to null terms */
static void trim(char** s) {
    /* find the first null terminator so we can work backwards. */
    char* end = *s;
    while (*end) {
        end++;
    }
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
int compare_symbols(char* a, char* b) {
    while (*a && *b) {
        /* stop when we've reached the end of one of the symbols */
        if ((*a == ',' || *a == delim) && !*b) break;
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
    return !*b && (*a == ',' || *a == delim || *a <= 0x20);
}

/* find ID of symbol in a symbols table */
int index_of_symbol(char* s, SymTable* syms) {
    int i = 0;
    for (i = 0; i < syms->len; i++) {
        if (compare_symbols(s, syms->table[i])) {
            return i;
        }
    }
    return -1;
}

/* walk to the end of the next symbol, adding it to the symbol table if it
 * hasn't been seen before. (and store the index to that symbol) */ 
static char* walk_symbol(char* s, int* id, SymTable* syms) {
    s = walk_whitespace(s);
    *id = index_of_symbol(s, syms);
    if (*id > -1) {
        /* we've seen this symbol before, so just walk to the end of it 
         * (when we see a delimiter or end of fact syntax) */
        while (s[0] && s[0] != delim && s[0] != ',') s++;
        return s;
    }
    
    /* new symbol found! Woo! */
    /* assign the next symbol in the symbols table to the current position of
     * the symbol names string. */
    syms->table[syms->len] = &(syms->names[syms->names_len]);
    syms->len = syms->len + 1;
    /* write the symbol string to the names array */
    while (s[0] && s[0] != delim && s[0] != ',') {
        syms->names[syms->names_len] = s[0];
        syms->names_len++;
        /* skip anything more than one whitespace. TODO: should eventually be
         * sep pass */
        if (*s == ' ') {
            s = walk_whitespace(s) - 1;
            /* syms->names[syms->names_len] = ' '; */
            /* syms->names_len++; */
        }
        *s++;
    }
    syms->names_len++; /* increment once more to ensure a null term between names */
    /* trim any whitespace off the end TODO: this should eventually be sep 
     * pass */
    trim(&syms->table[syms->len - 1]);
    *id = syms->len - 1;
    return s;
}

static char* walk_rule(char* s, RuleTable* rules) {
    int sym_id; /* used to track symbol count */
    int still_parsing_side = 1;
    /* process left-hand side, the rule condition. */
    while(still_parsing_side) {
        s = walk_symbol(s, &sym_id, rules->syms);
        rules->table[rules->len * rules->syms->max_len * 2 + sym_id]++;
        if (s[0] == ',') 
            s++;
        else 
            still_parsing_side = 0;
    }
    /* process right-hand side, the rule results. */
    /* we should be at a delimiter, indicating end of condition/start of results */
    if (s[0] != delim)
        printf("Broken rule?!\n"); /* TODO: figure out better way to do error reporting */
    s++;
    s = walk_whitespace(s);
    still_parsing_side = s[0] != delim;
    while (still_parsing_side) {
        s = walk_symbol(s, &sym_id, rules->syms);
        rules->table[rules->len * rules->syms->max_len * 2 + sym_id + rules->syms->max_len]++;
        if (s[0] == ',')
            s++;
        else
            still_parsing_side = 0;
    }
    rules->len++;
    return walk_whitespace(s);
}


/* STRT: maybe to get closer to what wryl said about typing being
 * <List<Pair<List<Symbol>, List<Symbol>>> (the implication being that facts are
 * just rules with blank lhs), should parse facts and rules simultaneously, both
 * into the rules table? The data struct is already set up for this to work,
 * just make facts not impact an acc, and instead do a separate loop through the
 * rules table to find empty rule lhs and increment acc there. */


static char* walk_fact(char* s, RuleTable* rules) {
    int sym_id;
    int still_parsing = 1;
    while (still_parsing) {
        s = walk_symbol(s, &sym_id, rules->syms);
        rules->table[rules->len * rules->syms->max_len * 2 + sym_id + rules->syms->max_len]++; /* TODO: this is what is breaking */
        if (s[0] == ',')
            s++;
        else
            still_parsing = 0;
    }
    rules->len++;
    return s;
}

int parse(char* s, RuleTable* rules) {
    /* the rule delimiter is the first character in the source.
     * conventionally '|', but can be anything.
     * (spacer glyph is the terminology used in
     * https://wiki.xxiivv.com/site/vera.html) */
    delim = s[0];
    s = walk_whitespace(s);
    while (s[0]) {
        s = walk_whitespace(s);
        if (s[0] == delim) {
            if (s[1] == delim) {
                /* if we find another delimiter immediately after, we know it's a
                 * fact, e.g. `|| this is a fact` */
                s = walk_fact(s + 2, rules);
            } else {
                /* instead of a rule which starts with a single delimiter, e.g.
                 * `|this is a condition| this is the result` */
                s = walk_rule(s + 1, rules);
            }
        } else if (s) {
            printf("Unexpected ending: [%c]%s]\n", s[0], s);
            return 0;
        }
    }
    return 1;
}
