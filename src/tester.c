#include <stdio.h>
#include "parser.h"

#define SRC_SZ 32768 /* maximum size of input vera source code */
#define NAM_SZ 32768 /* maximum combined text size of all symbol names? */
#define SYM_SZ 256  /* maximum number of unique symbols? */
#define RUL_SZ 128   /* maximum number of rules we can handle */

static char src[SRC_SZ];
static char names[NAM_SZ];
static char* _names = names;
static char* syms[SYM_SZ];
static char** _syms = syms;
static int* rules[RUL_SZ][SYM_SZ * 2];


static SymTable sym_table = {
    .names = names,
    .table = syms,
    .len = 0,
    .max_len = SYM_SZ,
    .names_len = 0,
};

static RuleTable rule_table = {
    .syms = &sym_table,
    .table = rules[0],
    .len = 0,
    .max_len = RUL_SZ,
};



static void print_all_symbols() {
    printf("-------\n");
    int i;
    for (i = 0; i < SYM_SZ; i++) {
        if (syms[i]) {
            printf("SYM %d:%s\n", i, syms[i]);
        }
    }
}

int main(int argc, char* argv[]) {

    printf("-----\n");
    puts(names);
    names[0] = 'h';
    names[1] = 'i';
    puts(names);
    char* n = names;
    puts(n);
    *n = 'n';
    n[0] = 'r';
    puts(n);
    
    puts(sym_table.names);
    SymTable* mysyms = &sym_table;
    puts(mysyms->names);

    mysyms->names[0] = 'o';
    puts(mysyms->names);
    printf("%s\n", mysyms->names);
    printf("-----\n");


    
    FILE *f;
    int a = 1;
    if(argc < 2)
        return !printf(
                "Vera, 6 Dec 2024.\nusage: vera input.vera\n");
    if(!(f = fopen(argv[a], "r")))
        return !printf("Source missing: %s\n", argv[a]);
    if(!fread(&src, 1, SRC_SZ, f))
        return !printf("Source empty: %s\n", argv[a]);



    /* SymTable sym_table = { */
    /*     .names = _names, */
    /*     .table = _syms, */
    /*     .len = 0, */
    /*     .max_len = SYM_SZ, */
    /*     .names_len = 32768, */
    /* }; */

    /* SymTable* mysyms = &sym_table; */

    printf("Initial addr: %d\n", names);
    printf("Initial: %s\n", &names[0]);
    printf("Initial: %s\n", &mysyms->names[0]);

    mysyms->names[0] = 'h';
    mysyms->names[1] = 'i';
    /* names[0] = 'h'; */
    /* names[1] = 'i'; */
    /* *_names = 'h'; */
    /* _names++; */
    /* *_names = 'i'; */
    /* _names--; */
    
    printf("Initial (safer?): %s\n", &_names);
    printf("Initial (safer?): %s\n", &_names[0]);
    printf("Initial: %s\n", &mysyms->names[0]);
    
    if(parse(src, &rule_table)) {
        if (argv[2] == "symbols") {
            print_all_symbols();
        }
    }
    else {
        return 1;
    }
    return 0;
}
