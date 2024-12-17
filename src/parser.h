/* ====================================================================
Copyright (c) 2024 Devine Lu Linvega
Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

/* The parser functions are responsible for turning vera source code into
 * rule and symbol tables, tracking the counts of symbols involved on rule
 * LHS/RHS */

/* ----------------------------------------------
Table relating internal symbols as pointers to where their string names are,
so if source is "apple, some fruit":

names = ['a', 'p', 'p', 'l', 'e', 0x0, 's', 'o', 'm', 'e', ' ', 'f', 'r', 'u', 'i', 't', 0x0]
table = [(pointer to names[0]), (pointer to names[6])]
---------------------------------------------- */
typedef struct SymTable {
    /* pointer to an array of all unique symbol names separated by null terms */
    char* names; 
    /* pointer to an array of pointers, the latter of which is each into the sym
     * names array. */
    char** table;
    int len; /* current number/position of syms in table */
    int max_len; /* bounds for the syms table */
    /* the last stop within the names string array, start next new symbol here. */
    int names_len;
    /* int max_names_len */
} SymTable;

/* ----------------------------------------------
Table where each row has counts of symbols on LHS and counts of symbols on RHS of a rule,
so if source was: (given symbol table example above)

|apple, apple| some fruit
|| apple, apple

table = [2, 0, 0, 1, 0, 0, 2, 0]
         ^LHS0 ^RHS0 ^LHS1 ^RHS1

Note that I'm assuming a single dimensional rules table, do math manually
because it's too weird and annoying to figure out how to correctly pass around
a two dimensional array of unknown dimensions.
---------------------------------------------- */
typedef struct RuleTable {
    SymTable* syms;
    int* table;
    int len; /* current number/position of rules in table */
    int max_len; /* bounds for the rules table */
} RuleTable;

/* check if two passed symbols are the same */
int compare_symbols(char* a, char* b);

/* find ID of symbol in a symbols table */
int index_of_symbol(char* s, SymTable* syms);

int parse(char* s, RuleTable* rules);
