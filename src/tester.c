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
#include "variables_pass.h"

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

    int print_symbols = 0; /* --psymbols */
    int print_rules = 0; /* --prules */
    int implicit_constants = 1; /* --no-implicit-constants */
    int vars_pass = 0; /* --vars */
    int vars_force = 0; /* --force */
    int filename_argv_index = -1; /* if never set, expect stdin */

    /* cli arg parsing */
    while (a < argc) {
        if (strcmp(argv[a], "--psymbols") == 0)
            print_symbols = 1;
        else if (strcmp(argv[a], "--prules") == 0)
            print_rules = 1;
        else if (strcmp(argv[a], "--no-implicit-constants") == 0)
            implicit_constants = 0;
        else if (strcmp(argv[a], "--vars") == 0)
            vars_pass = 1;
        else if (strcmp(argv[a], "--force") == 0)
            vars_force = 1;
        else
            filename_argv_index = a;
        a++;
    }
    
    /* grab source code from correct source */
    if (filename_argv_index > -1) {
        /* open and read in the source file */
        if(!(f = fopen(argv[filename_argv_index], "r")))
            return !printf("Source missing: %s\n", argv[a]);
        if(!fread(&src, 1, SRC_SZ, f))
            return !printf("Source empty: %s\n", argv[a]);
    }
    else {
        /* read source code from stdin */
        /* NOTE: if you aren't piping anything in, you can just enter code and
         * use ctrl+D to term */
        fread(&src, 1, SRC_SZ, stdin);
    }

    if(parse(src, &rule_table, implicit_constants)) {
        if (vars_pass) {
            run_variables_pass(&rule_table, vars_force);
        }
        if (print_symbols)
            print_all_symbols();
        if (print_rules)
            print_all_rules();
    }
    else {
        return 1;
    }
    return 0;
}
