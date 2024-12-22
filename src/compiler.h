/* ====================================================================
Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

/* Turn vera code into library in other langs */

#ifndef COMPILER_H
#define COMPILER_H

#include "parser.h"
#include "interpreter.h"

void compile_to_c(RuleTable* rules, BagOfFacts* bag, char* src_out);

#endif
