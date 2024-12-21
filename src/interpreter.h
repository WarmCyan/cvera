/* ====================================================================
Original Copyright (c) 2024 Devine Lu Linvega
Modified Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

/* The interpreter handles "running" a set of rules, tracking an accumulator or
 * bag of facts/symbols over time. */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

typedef struct BagOfFacts {
    SymTable* syms;
    /* array of numbers, each referring to the number of that index's symbol
     * "currently in play" */
    int* accumulator;
} BagOfFacts;

/* Find all of the facts in the rules table, rules with no LHS */
void populate_facts(BagOfFacts* bag, RuleTable* rules);

/* Find the next rule and applies it, returns the index of the rule matched or
 * -1 if no matches were found */
int step(BagOfFacts* bag, RuleTable* rules);

/* Pass max_steps of -1 to run until halt (no more rules matched). Returns the
 * number of steps taken */
/* TODO: it would be neat to count number of times each rule is matched, could
 * allow visualizing a sort of heatmap */
int eval(BagOfFacts* bag, RuleTable* rules, int max_steps);

#endif
