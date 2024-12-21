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

#define SRC_SZ 32768 /* maximum size of input vera source code */
#define NAM_SZ 32768 /* maximum combined text size of all symbol names? */
#define SYM_SZ 256  /* maximum number of unique symbols? */
#define RUL_SZ 128   /* maximum number of rules we can handle */

static char src[SRC_SZ];
static char names[NAM_SZ];
static char* syms[SYM_SZ];
static int rules[RUL_SZ * SYM_SZ * 2];


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



static void print_all_symbols() {
    int i;
    for (i = 0; i < SYM_SZ; i++) {
        if (syms[i]) {
            printf("SYM %d:%s\n", i, syms[i]);
        }
    }
}

static void
print_all_rules() {
    int i, j;
    for (i = 0; i < RUL_SZ; i++) {
        int sum = 0;
        /* determine if this rule "exists"/isn't blank if any of the associated symbol
           s are > 0 */
        for (j = 0; j < SYM_SZ*2; j++) {
            sum += rules[i*SYM_SZ*2 + j];
        }
        if (sum > 0) {
            printf("RUL %d:|", i);
            int first_printed = 0; /* use this to determine whether to print, or not */
            for (j = 0; j < SYM_SZ; j++) {
                /* handle printing symbol and multiplicity if part of the rule
                 * lhs */
                if (rules[i*SYM_SZ*2 + j] == 1) {
                    if (first_printed) printf(",");
                    first_printed = 1;
                    printf("%s", syms[j]);
                } else if (rules[i*SYM_SZ*2 + j] > 1) {
                    if (first_printed) printf(",");
                    first_printed = 1;
                    printf("%s:%d", syms[j], rules[i*SYM_SZ*2 + j]);
                }
            }
            printf("|");
            first_printed = 0;
            int rel_j;
            for (j = SYM_SZ; j < SYM_SZ*2; j++) {
                /* handle printing symbol and multiplicity if part of the rule
                 * rhs */
                rel_j = j - SYM_SZ;
                if (rules[i*SYM_SZ*2 + j] == 1) {
                    if (first_printed) printf(",");
                    first_printed = 1;
                    printf("%s", syms[rel_j]);
                } else if (rules[i*SYM_SZ*2 + j] > 1) {
                    if (first_printed) printf(",");
                    first_printed = 1;
                    printf("%s:%d", syms[rel_j], rules[i*SYM_SZ*2 + j]);
                }
            }
            printf("\n");
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
        if (argc > 2) {
            if (strcmp(argv[2], "--psymbols") == 0) {
                print_all_symbols();
            }
            if (strcmp(argv[2], "--prules") == 0) {
                print_all_rules();
            }
        }
    }
    else {
        return 1;
    }
    return 0;
}
