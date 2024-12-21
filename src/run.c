/* ====================================================================
Original Copyright (c) 2024 Devine Lu Linvega
Modified Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "interpreter.h"

#define SRC_SZ 32768 /* maximum size of input vera source code */
#define NAM_SZ 32768 /* maximum combined text size of all symbol names? */
#define SYM_SZ 256  /* maximum number of unique symbols? */
#define RUL_SZ 128   /* maximum number of rules we can handle */

static char src[SRC_SZ];
static char names[NAM_SZ];
static char* syms[SYM_SZ];
static int rules[RUL_SZ * SYM_SZ * 2];
static int acc[SYM_SZ];

static SymTable sym_table = {
    .names = names,
    .table = syms,
    .len = 0,
    .max_len = SYM_SZ,
    .names_len = 0,
};

static RuleTable rule_table = {
    .syms = &sym_table,
    .table = rules,
    .len = 0,
    .max_len = RUL_SZ,
};

static BagOfFacts bag = {
    .syms = &sym_table,
    .accumulator = acc,
};

static void print_bag() {
    int i;
    for (i = 0; i < SYM_SZ; i++) {
        if (acc[i] == 1)
            printf("%s\n", syms[i]);
        if (acc[i] > 1) {
            printf("%s:%d\n", syms[i], acc[i]);
        }
    }
}

int main(int argc, char* argv[]) {
    FILE *f;
    int a = 1;
    if(argc < 2)
        return !printf(
                "Vera, 6 Dec 2024.\nusage: vera input.vera\n");
    if(!(f = fopen(argv[a], "r")))
        return !printf("Source missing: %s\n", argv[a]);
    if(!fread(&src, 1, SRC_SZ, f))
        return !printf("Source empty: %s\n", argv[a]);

    if(parse(src, &rule_table)) {
        populate_facts(&bag, &rule_table);

        if (argc > 2) {
            if (strcmp(argv[2], "--plast") == 0) {
                eval(&bag, &rule_table, 5);
                print_bag();
            }
        }
        else {
            int out = 0;
            while (out != -1) {
                print_bag();
                out = step(&bag, &rule_table);
                printf("Matched rule %d...\n", out);
            /* eval(&bag, &rule_table, 5); */
            }
            print_bag();
        }
    }
    else {
        return 1;
    }
    return 0;
}
