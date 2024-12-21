/* ====================================================================
Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

#include "variables_pass.h"


/* return nonzero if "a sep b" == "z" */
int compare_syms_concat_separator_symbol(char* a, char* b, char* sep, char* z) {
    int not_equal = 0;
    /* make sure the a symbol part matches the beginning of the z symbol */
    while (*a && *z) {
        if (*a != *z) return 0;
        a++;
        z++;
    }
    /* if we've reached the end of z, this isn't a match */
    if (!*z)
        return 0;
    /* make sure the separator (' -> ') portion aligns */
    while (*sep && *z) {
        if (*sep != *z) return 0;
        sep++;
        z++;
    }
    /* if we've reached the end of z, this isn't a match */
    if (!*z)
        return 0;
    /* make sure the b symbol part matches the end of the z symbol */
    while (*b && *z) {
        if (*b != *z) return 0;
        b++;
        z++;
    }
    /* we should now be at the end of both symbols */
    return !*b && !*z;
}

/* add "a sep b" symbol to the passed symbols table, return index of new symbol
 * in table. */
int add_syms_concat_separator_symbol(char* a, char* b, char* sep, SymTable* syms) {
    /* assign the next symbol in the symbols table to the current position of
     * the symbol names string. */
    syms->table[syms->len] = &(syms->names[syms->names_len]);
    syms->len = syms->len + 1;
    /* write the symbol string to the names array */
    while (a[0]) {
        syms->names[syms->names_len] = a[0];
        syms->names_len++;
        *a++;
    }
    while (sep[0]) {
        syms->names[syms->names_len] = sep[0];
        syms->names_len++;
        *sep++;
    }
    while (b[0]) {
        syms->names[syms->names_len] = b[0];
        syms->names_len++;
        *b++;
    }
    syms->names_len++; /* increment once more to ensure a null term between names */
    return syms->len - 1;
}

/* destructive data movement rules of form:
 * |a -> b, a| a -> b, b
 * |a -> b|
 * with force = 0, a rule doesn't get added if it won't be needed, pass
 * something non-zero to force all rules to be generated (useful e.g. for a
 * repl)
 * */
void add_move_a_to_b_rules(int sym_a_index, int sym_b_index, RuleTable* rules, int force) {
    /* search for the 'a -> b' symbol - if it's not in the symbols table yet,
     * that means we don't need to add this rule, because it wouldn't be used? (if not force) */
    int movement_sym_index = -1;
    int i; /* symbol index */
    for (i = 0; i < rules->syms->len; i++) {
        if (compare_syms_concat_separator_symbol(rules->syms->table[sym_a_index], rules->syms->table[sym_b_index], " -> ", rules->syms->table[i])) {
            movement_sym_index = i;
        }
    }

    if (movement_sym_index == -1) {
        if (force) {
            /* add the 'a -> b' name to symbol names list if not already there */
            movement_sym_index = add_syms_concat_separator_symbol(
                    rules->syms->table[sym_a_index],
                    rules->syms->table[sym_b_index],
                    " -> ",
                    rules->syms);
        }
        else return;
    }

    /* now add the actual rules */

    /* add the |a -> b, a| a -> b, b rule */
    /* left hand side a -> b */
    rules->table[rules->len * rules->syms->max_len * 2 + movement_sym_index] = 1;
    /* left hand side a */
    rules->table[rules->len * rules->syms->max_len * 2 + sym_a_index] = 1;
    /* right hand side a -> b */
    rules->table[rules->len * rules->syms->max_len * 2 + movement_sym_index + rules->syms->max_len] = 1;
    /* right hand side b */
    rules->table[rules->len * rules->syms->max_len * 2 + sym_b_index + rules->syms->max_len] = 1;
    rules->len++;

    /* add the |a -> b| rule */
    rules->table[rules->len * rules->syms->max_len * 2 + movement_sym_index] = 1;
    rules->len++;
}

/* nondestructive data movement rules of form:
 * |a = b, a| a = b, b_copy, a
 * |a = b|
 * |b_copy| b */
void add_copy_a_to_b_rules(int sym_a_index, int sym_b_index, RuleTable* rules) {
}


void run_variables_pass(RuleTable* rules, int force_all_rules) {
    /* search for annotation rules '|#|...' */
    int annotation_sym_index = index_of_symbol("#", rules->syms);
    int variables_sym_index = index_of_symbol("variables", rules->syms);
    if (annotation_sym_index == -1 || variables_sym_index == -1) {
        /* if there's no variable annotation symbols, nothing to do */
        return;
    }

    /* collect all variable symbols */
    /* TODO: just limiting to 10 for now, prob need to figure out better way to
     * do this */
    int variable_symbols[10];
    int vars_index = 0;
    int i; /* rule index */
    int j; /* symbol index */
    for (i = 0; i < rules->len; i++) {
        if (rules->table[i * rules->syms->max_len * 2 + annotation_sym_index] &&
                rules->table[i * rules->syms->max_len * 2 + variables_sym_index + rules->syms->max_len]) {
            /* we found a |#| variables annotation! Add all other symbols found
             * on RHS of this rule to variable_symbols */
            for (j = 0; j < rules->syms->len; j++) {
                if (j == variables_sym_index) continue; /* ignore the "variables" symbol itself obviously */
                if (rules->table[i * rules->syms->max_len * 2 + j + rules->syms->max_len]) {
                    variable_symbols[vars_index] = j;
                    vars_index++;
                }
            }
        }
    }
    
    /* Add the appropriate rules for each one */
    /* i is now symbol a, j is now symbol b */
    for (i = 0; i < vars_index; i++) {
        for (j = 0; j < vars_index; j++) {
            /* doesn't make sense to add a -> a rules... */
            if (i == j) continue;
            add_move_a_to_b_rules(variable_symbols[i], variable_symbols[j], rules, force_all_rules);
        }
    }
}
