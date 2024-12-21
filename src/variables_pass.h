/* ====================================================================
Copyright (c) 2024 Nathan Martindale

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
==================================================================== */

/* See https://wiki.nova-lang.net/index.php?title=Variable_Annotations */

#ifndef VARIABLES_PASS_H
#define VARIABLES_PASS_H

#include "parser.h"

void run_variables_pass(RuleTable* rules, int force_all_rules);

#endif
