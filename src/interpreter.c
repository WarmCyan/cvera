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
#include "interpreter.h"

/* Find all of the facts in the rules table, rules with no LHS */
void populate_facts(BagOfFacts* bag, RuleTable* rules) {
    int i, j;
    for (i = 0; i < rules->len; i++) {
        /* if any symbols on this side of the rule are nonzero, it's not a fact. */
        int empty_lhs = 1;
        for (j = 0; j < bag->syms->len; j++) {
            if (rules->table[i * rules->syms->max_len * 2 + j]) { /* found non-zero */
                empty_lhs = 0;
                break;
            }
        }
        if (empty_lhs) {
            /* if this is a fact, add all rhs symbols to the accumulator */
            /* NOTE: seems like here we do care about multiplicity? */
            for (j = 0; j < rules->syms->len; j++) {
                if (rules->table[i * rules->syms->max_len * 2 + j + rules->syms->max_len])
                    bag->accumulator[j] += rules->table[i * rules->syms->max_len * 2 + j + rules->syms->max_len];
            }
        }
    }
}


/* Check if the bag has sufficient facts to trigger on the passed row */
/* NOTE: multiplicity of symbols in the condition of a rule doesn't matter? */
/* returns the number of times this rule could execute */
/* EXAMPLE:
 * ||x:5  (this means any rule taking just x could run 5 times)
 * ||y:4  (any rule taking just y could run 4 times)
 * |x,y|z (limited by y, so should end up with z:4 and x:1)
 */
static int check_rule_against_accumulator(int* rule_row, BagOfFacts* bag) {
    int i;
    /* we check the left hand side of the rule - any rule symbol requirements
     * not in the bag means this rule isn't a match. */
    /* NOTE: we don't need to use max_len since any space between len/max_len
     * isn't used by definition */
    int lhs_sum = 0;
    int executions = -1; /* number of times this rule could execute, based on smallest
                           matching quantity within accumulator */
    for (i = 0; i < bag->syms->len; i++) {
        if (rule_row[i]) {
            /* if symbol i is > 0 in rule LHS and 0 in the accumulator, this is
             * not a match. */
            if (!bag->accumulator[i])
                return 0;
            /* we get a lhs sum because we don't want to match a "fact" here, there
             * needs to be _some_ condition. */
            lhs_sum += rule_row[i];
            /* does this accumulator value limit the number of times we can run? */
            if (executions == -1 || bag->accumulator[i] < executions)
                executions = bag->accumulator[i];
        }
    }
    if (lhs_sum == 0) return 0;
    return executions;
}

/* Find the next rule and applies it, returns the index of the rule matched or
 * -1 if no matches were found */
int step(BagOfFacts* bag, RuleTable* rules) {
    int i, j;
    int executions; /* BEHOLD THE LORD HIGH EXECUTIONER */
    for (i = 0; i < rules->len; i++) {
        executions = check_rule_against_accumulator(&(rules->table[i * rules->syms->max_len * 2]), bag);
        if (executions > 0) {
            for (j = 0; j < rules->syms->len; j++) {
                /* Remove LHS facts from the accumulator */
                if (rules->table[i * rules->syms->max_len * 2 + j])
                    bag->accumulator[j] -= executions;
                /* Add any RHS facts to the accumulator */
                if (rules->table[i * rules->syms->max_len * 2 + j + rules->syms->max_len])
                    /* the executions*table because in |x|x:2, we only execute
                     * once but we add two x's. */
                    bag->accumulator[j] += executions * rules->table[i * rules->syms->max_len * 2 + j + rules->syms->max_len];
            }
            return i;
        }
    }
    return -1;
}

/* Pass max_steps of -1 to run until halt (no more rules matched). Returns the
 * number of steps taken. */
/* TODO: it would be neat to count number of times each rule is matched, could
 * allow visualizing a sort of heatmap */
int eval(BagOfFacts* bag, RuleTable* rules, int max_steps) {
    int steps = 0;
    int last_rule_match = 0;
    while (last_rule_match != -1) {
        steps += 1;
        last_rule_match = step(bag, rules);
        if (max_steps != -1 && steps >= max_steps) break;
    }
    return steps;
}
