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
#include "variables_pass.h"

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

/* this is a printout of the same format as the DEBUG compiled c version */
static void printout() {
    int i;
    for (i = 0; i < sym_table.len; i++) {
        printf("%d,", acc[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    FILE *f;
    int a = 1;

    int print_last_only = 0; /* --plast */
    int printout_format = 0; /* --printout */
    int implicit_constants = 1; /* --no-implicit-constants */
    int max_steps = -1; /* --steps [NUM] */
    int vars_pass = 0; /* --vars */
    int filename_argv_index = -1; /* if never set, expect stdin */

    /* cli arg parsing */
    while (a < argc) {
        if (strcmp(argv[a], "--plast") == 0)
            print_last_only = 1;
        else if (strcmp(argv[a], "--no-implicit-constants") == 0)
            implicit_constants = 0;
        else if (strcmp(argv[a], "--steps") == 0) {
            a++;
            walk_number(argv[a], &max_steps);
        }
        else if (strcmp(argv[a], "--vars") == 0)
            vars_pass = 1;
        else if (strcmp(argv[a], "--printout") == 0)
            printout_format = 1;
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
            run_variables_pass(&rule_table, 0);
        }
        populate_facts(&bag, &rule_table);

        if (print_last_only) {
            eval(&bag, &rule_table, max_steps);
            print_bag();
        }
        else if (printout_format) {
            eval(&bag, &rule_table, -1);
            printout();
        }
        else {
            int out = 0;
            int steps_to_take = max_steps;
            while (out != -1 && (max_steps == -1 || steps_to_take > 0)) {
                print_bag();
                out = step(&bag, &rule_table);
                printf("Matched rule %d...\n", out);
                steps_to_take -= 1;
            }
            print_bag();
        }
    }
    else {
        return 1;
    }
    return 0;
}
