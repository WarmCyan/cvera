/* ====================================================================
Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

#include "compiler.h"
#include <stdio.h>

static char* add_string(char* str, char* cursor) {
    while (*str) {
        *cursor = *str;
        cursor++;
        str++;
    }
    return cursor;
}

/* clean up symbol names for use as variable names */
static char* add_clean_var_str(char* str, char* cursor) {
    if (*str && str[0] == '<') {
        cursor = add_string("_o_", cursor);
        str++;
    }
    else if (*str && str[0] == '>') {
        cursor = add_string("_i_", cursor);
        str++;
    }
    else if (*str && str[0] == '-') {
        cursor = add_string("minus_", cursor);
        str++;
    }
    while (*str) {
        /* '->' into 'to' */
        if (str[0] == '-' && str[1] == '>') {
            *cursor = 't';
            cursor++;
            *cursor = 'o';
            str++;
        }
        /* '#' into '__', at least until we add dead code pass */
        else if (*str == '#') {
            *cursor = '_';
            cursor++;
            *cursor = '_';
        }
        else if (*str == ' ' || *str == '-' || *str == '>' || *str == ':' || *str == '\n' || *str == '.' || *str == '\'') {
            *cursor = '_';
        }
        else *cursor = *str;
        cursor++;
        str++;
    }
    return cursor;
}

static char* add_num_to_str(int num, char* cursor) {
    sprintf(cursor, "%d", num);
    while (*cursor) cursor++; /* sprintf doesn't move cursor, so catch back up */
    return cursor;
}

/* use bag just so we know what to default assign to vars in their definitions */
void compile_to_c(RuleTable* rules, BagOfFacts* bag, char* src_out) {
    char* cursor = src_out;

    /* add the MIN define */
    cursor = add_string("#define MIN(x, y) (((x) < (y)) ? (x) : (y))\n\n", cursor);



    

    /* output all the static int symbols */
    int i; /* symbol index */
    for (i = 0; i < rules->syms->len; i++) {
        cursor = add_string("static int ", cursor);
        cursor = add_clean_var_str(rules->syms->table[i], cursor);
        cursor = add_string(" = ", cursor);
        cursor = add_num_to_str(bag->accumulator[i], cursor);
        cursor = add_string(";\n", cursor);
    }

    /* add the executions int */
    cursor = add_string("\nint executions = 0;\n\n", cursor);





    

    /* add the step function */
    cursor = add_string("int step() {\n", cursor);
    /* i is now rule index */
    int j; /* symbol index */ 
    int k; /* condition index (lhs symbol of rule) */
    int first_rule_added = 0;
    int num_rules_added = 0;
    for (i = 0; i < rules->len; i++) {
        /* add each rule as a conditional */

        /* && all the lhs vars */
        /* (first find all lhs symbols) */
        int lhs_symbol_indices[rules->syms->len]; /* can't have more than every symbol in the lhs of a rule :D */
        int distinct_lhs_symbol_count = 0; /* we need this to determine num nested MIN calls */
        for (j = 0; j < rules->syms->len; j++) {
            if (rules->table[i * rules->syms->max_len * 2 + j]) {
                lhs_symbol_indices[distinct_lhs_symbol_count] = j;
                distinct_lhs_symbol_count++;
            }
        }
        lhs_symbol_indices[distinct_lhs_symbol_count] = -1; /* indicate at end? may not be necessary */
        if (distinct_lhs_symbol_count < 1) continue;
        cursor = add_string("\t", cursor);

        /* handle if vs else if */
        if (num_rules_added > 0) cursor = add_string("else ", cursor);
        cursor = add_string("if (", cursor);

        /* now and && them */
        cursor = add_clean_var_str(rules->syms->table[lhs_symbol_indices[0]], cursor);
        for (k = 1; k < distinct_lhs_symbol_count; k++) {
            cursor = add_string(" && ", cursor);
            cursor = add_clean_var_str(rules->syms->table[lhs_symbol_indices[k]], cursor);
        }
        cursor = add_string(") {\n", cursor);

        /* compute number of executions via nested MIN */
        cursor = add_string("\t\texecutions = ", cursor);
        if (distinct_lhs_symbol_count == 1) {
            cursor = add_clean_var_str(rules->syms->table[lhs_symbol_indices[0]], cursor);
        }
        else {
            /* one less MIN( than there are conditions */
            for (k = 0; k < distinct_lhs_symbol_count - 1; k++) {
                cursor = add_string("MIN(", cursor);
            }
            /* now add first condition symbol */
            cursor = add_clean_var_str(rules->syms->table[lhs_symbol_indices[0]], cursor);
            /* then iterate through the rest adding ", symbol)" */
            for (k = 1; k < distinct_lhs_symbol_count; k++) {
                cursor = add_string(", ", cursor);
                cursor = add_clean_var_str(rules->syms->table[lhs_symbol_indices[k]], cursor);
                cursor = add_string(")", cursor);
            }
        }
        cursor = add_string(";\n", cursor);

        /* for each lhs var, subtract executions */
        for (k = 0; k < distinct_lhs_symbol_count; k++) {
            cursor = add_string("\t\t", cursor);
            cursor = add_clean_var_str(rules->syms->table[lhs_symbol_indices[k]], cursor);
            cursor = add_string(" -= executions;\n", cursor);
        }

        /* for each rhs var, add executions */
        for (j = 0; j < rules->syms->len; j++) {
            if (rules->table[i * rules->syms->max_len * 2 + j + rules->syms->max_len]) {
                cursor = add_string("\t\t", cursor);
                cursor = add_clean_var_str(rules->syms->table[j], cursor);
                cursor = add_string(" += executions;\n", cursor);
            }
        }

        /* return rule index */
        cursor = add_string("\t\treturn ", cursor);
        cursor = add_num_to_str(num_rules_added, cursor);
        cursor = add_string(";\n", cursor);
        
        cursor = add_string("\t}\n", cursor);
        num_rules_added++;
    }
    cursor = add_string("\treturn -1;\n}\n", cursor);


    
    /* add the eval function */
    cursor = add_string("\nvoid eval() {\n\tint out = 0;\n\twhile (out != -1) {\n\t\tout = step();\n\t}\n}", cursor);
        /* run step until return is not -1 */

    

    
    /* add debug option */
    cursor = add_string("\n\n#ifdef DEBUG\n", cursor);
    
    cursor = add_string("\n#include <stdio.h>\n", cursor);
    
    /* add printout func */
    cursor = add_string("\nvoid printout() {\n", cursor);
    for (i = 0; i < rules->syms->len; i++) {
        cursor = add_string("\tprintf(\"\%d,\", ", cursor);
        cursor = add_clean_var_str(rules->syms->table[i], cursor);
        cursor = add_string(");\n", cursor);
    }
    cursor = add_string("\tprintf(\"\\n\");\n}\n", cursor);

    /* add main func */
    cursor = add_string("\nint main() {\n\teval();\n\tprintout();\n}\n", cursor);
    
    cursor = add_string("\n#endif\n", cursor);
}
