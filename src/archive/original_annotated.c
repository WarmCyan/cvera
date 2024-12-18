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

#define SRC_SZ 0x8000 /* maximum size of input vera source code */
#define DIC_SZ 0x8000 /* maximum combined text size of all symbol names? */
#define SYM_SZ 0x100  /* maximum number of unique symbols? */
#define RUL_SZ 0x80   /* maximum number of rules we can handle */

/* src is the full input source code */
static char spacer_glyph, src[SRC_SZ];
/* ahhh the secondary pointer definitions are because arrays aren't technically
 * pointers, at least wrt to incrementing, see https://stackoverflow.com/questions/4607128/in-c-are-arrays-pointers-or-used-as-pointers
 * (incrementing a pointer moves by 4 bytes, I assume incrementing an array does
 * not? or maybe you just want a pointer separate from the array to be able to
 * iterate/index into it without losing the pointer to the beginning of the
 * array? */
/* TODO: I'm still not actually sure what dict is */
static char dict[DIC_SZ], *_dict = dict;
/* So....dict is an array of all unique symbol names separated by null terms, and each
 * entry in syms then is a reference into a point within that dict? */
static char *syms[SYM_SZ], **_syms = syms;
/* acc is the current bag of facts, stands for accumulator, is the current count
 * of each symbol */
/* presumably each rule has a count of each symbol in lhs vs rhs (the *2 part?)
*/
static int acc[SYM_SZ], rules[RUL_SZ][SYM_SZ * 2], _rules = 0;

/* iterate through source until we find a non-whitespace character */
static char *
walk_whitespace(char *s) {
    /* I added this s - src < SRC_SZ, because without I kept getting
     * segfaults after removing the check for line feed in walk_symbol, but now
     * we're getting different registers at the end than original.c */
    /* Ah, so no, this isn't the correct place to add that: s could be a
     * pointer into dict, which is after src, so this check would always
     * immediately return. I instead put it down in walk_symbol. */
    printf("(");
    while(s[0] && s[0] <= 0x20 /* && s - &src[0] < SRC_SZ*/) {
        printf("'%c`", s[0]);
        s++;
    }
    printf(")");
    printf("[%02x]", s[0]);
    return s;
}

/* find the next null terminator. */
static char *
where_is_null(char *s) {
    while(*s) s++;
    return s;
}

/* this is only being called on new symbols in walk_symbol */
static void
trim(char **s) {
    char *cap = where_is_null(*s) - 1;
    /* work backwards from null terminator until we find a real letter,
     * turning any whitespace into moar null terms. */
    while(*cap) {
        if(*cap > 0x20) break;
        *cap = 0;
        cap--;
    }
}

/* check if the two passed symbols are the same */
static int
symbol_compare(char *a, char *b) {
    printf("\tcompare:");
    while(*a && *b) {
        printf("%c:%c ", *a, *b);
        if((*a == ',' || *a == spacer_glyph) && !*b) break;
        if(*a != *b) break;
        /* ...for whatever reason, walking spaces on a always seems to go one
         * character too far?! and subtracting 1 fixes it, but I don't
         * understand, when printing the characters being skipped, that first
         * letter in new word doesn't show up... * don't understand, when
         * printing the characters being skipped, that first letter in new word
         * doesn't show up... */
        /* NOPE, this was also an issue with me putting the out of bounds check
         * in walk_whitespace instead of walk_symbol. So both a/b should be
         * decremented by 1 since the end of this block increments both
         * (otherwise you skip checking the first character after each space) */
        if(*a == ' ') a = walk_whitespace(a) - 1;
        if(*b == ' ') b = walk_whitespace(b) - 1;
        /* I think a potential problem is a/b aren't trimmed, so newlines in
         * the middle of symbols might break? so we fix by walking whitespace on
         * < 0x20 here (as long as not null term?) */
        /* ...this doesn't actually appear to be the case? Not 100% sure why,
         * maybe I just didn't construct a sufficient test for this */
        /*if (*a <= 0x20 && *a > 0x0) a = walk_whitespace(a);
          if (*b <= 0x20 && *b > 0x0) b = walk_whitespace(b);*/
        a++, b++;
    }
    /* two symbols are the same if we've made it this far and a is indeed at the
     * end of a symbol? (comma, spacer, or whitespace) */
    /* OH. I guess we're negating *b because if we iterated to the end of it,
     * it's gonna be a null term? so !*b should be non-null (thus true) and a
     * should be the end of a symbol in some way */
    printf("\n");
    return !*b && (*a == ',' || *a == spacer_glyph || *a <= 0x20);
}

static int
find_symbol_index(char *s) {
    int i, len = _syms - &syms[0];
    for(i = 0; i < len; i++) {
        if(symbol_compare(s, syms[i])) {
            printf("\tfound!\n");
            return i;
        }
    }
    printf("\tnew!\n");
    return -1;
}

static void
print_dict() {
    printf("########\n");
    int i;
    for (i = 0; i < DIC_SZ; i++) {
        if (dict[i]) {
            printf("DIC %d:", i);
            printf("%c\n", dict[i]);
        }
    }
}

/* Iterate forward until we hit the end of this symbol, which should be
 * delineated either by a comma, or by a spacer glyph (indicating the next rule
 * or fact.) Also I think the
 * reference to the symbol is put into id romehow? */
static char *
walk_symbol(char *s, int *id) {
    s = walk_whitespace(s);
    *id = find_symbol_index(s);
    if(*id > -1) {
        /* We hit this code if this is a symbol we've encountered before? */

        /* TODO: why are we explicitly checking for 0xa? That's a line feed
         * character? Does a newline always indicate a new symbol? Do we not
         * then need to also check for carriage return? Does `trim` take care
         * of that? */
        /* NOTE: I'm taking that out because I think a new line is only supposed
         * to "start a new symbol" if the end of the previous line was a comma,
         * so I think the comma and the spacer glyph are the only two characters
         * that should be delineating symbols. */
        /* This did indeed seem to be why tests 1 and 2 were failing. But, now
         * the original tests definitely don't line up :( */
        /* So THIS is where it makes sense to have an out of bounds check, this
         * is only running on src directly, and we seg fault (I think) if we go
         * too far */
        while(s[0] && s[0] != spacer_glyph && s[0] != ',') {
            printf("\tseeking symbol end\n");
            *s++;
        }
        return s;
    }
    *_syms = _dict;
    while(s[0] && s[0] != spacer_glyph && s[0] != ',') {
        *_dict++ = *s++;
        if(*s == ' ')
            s = walk_whitespace(s), *_dict++ = ' ';
    }
    trim(_syms);
    *_dict++ = 0, *id = _syms - &syms[0], _syms++;
    print_dict();
    return s;
}

/* Note that this returns next point within source code that isn't a
 * whitespace */
static char *
walk_rule(char *s) {
    printf("Walking rule...\n");
    int id, valid = 1;
    /* left-hand side, the rule condition. */
    while(valid) {
        s = walk_symbol(s, &id);
        rules[_rules][id]++;
        if(s[0] == ',')
            s++, valid = 1;
        else
            valid = 0;
    }
    /* right-hand side, the rule results. */
    /* we should be at a spacer glyph, indicating the end of condition/start of
     * results */
    if(s[0] != spacer_glyph)
        printf("Broken rule?!\n");
    s++;
    s = walk_whitespace(s);
    valid = s[0] != spacer_glyph;
    while(valid) {
        s = walk_symbol(s, &id);
        rules[_rules][id + SYM_SZ]++;
        if(s[0] == ',')
            s++, valid = 1;
        else
            valid = 0;
    }
    _rules++;
    return walk_whitespace(s);
}

static char *
walk_fact(char *s) {
    printf("Walking fact\n");
    int id, valid = 1;
    while(valid) {
        s = walk_symbol(s, &id), acc[id]++;
        if(s[0] == ',')
            s++, valid = 1;
        else
            valid = 0;
    }
    return s;
}


static void
print_all_symbols() {
    printf("-------\n");
    int i;
    for (i = 0; i < SYM_SZ; i++) {
        if (syms[i]) {
            printf("SYM %d:%s\n", i, syms[i]);
        }
    }
}

static void
print_all_rules() {
    printf("-------\n");
    int i, j;
    for (i = 0; i < RUL_SZ; i++) {
        int sum = 0;
        /* determine if this rule "exists"/isn't blank if any of the associated symbol
           s are > 0 */
        for (j = 0; j < SYM_SZ*2; j++) {
            sum += rules[i][j];
        }
        if (sum > 0) {
            printf("RUL %d:|", i);
            for (j = 0; j < SYM_SZ; j++) {
                /* handle printing symbol and multiplicity if part of the rule
                 * lhs */
                if (rules[i][j] == 1) {
                    printf("%s,", syms[j]);
                } else if (rules[i][j] > 1) {
                    printf("%s^%d,", syms[j], rules[i][j]);
                }
            }
            printf("|");
            int rel_j;
            for (j = SYM_SZ; j < SYM_SZ*2; j++) {
                /* handle printing symbol and multiplicity if part of the rule
                 * rhs */
                rel_j = j - SYM_SZ;
                if (rules[i][j] == 1) {
                    printf("%s,", syms[rel_j]);
                } else if (rules[i][j] > 1) {
                    printf("%s^%d,", syms[rel_j], rules[i][j]);
                }
            }
            printf("\n");
        }
    }
}

static int
parse(char *s) {
    /* the spacer glyph is the first character in the source.
     * conventionally '|', but can be anything.
     * (spacer glyph is the terminology used in
     * https://wiki.xxiivv.com/site/vera.html) */
    spacer_glyph = s[0];
    s = walk_whitespace(s);
    while(s[0]) {
        s = walk_whitespace(s);
        if(s[0] == spacer_glyph) {
            /* if we find another spacer glyph immediately after, we know
             * it's a fact, e.g. `|| this is a fact` */
            if(s[1] == spacer_glyph) {
                s = walk_fact(s + 2);
                /* instead of a rule which starts with a single spacer glyph
                 * e.g. `|this is a condition| this is the result` */
            } else
                s = walk_rule(s + 1);
            print_all_symbols();
            print_all_rules();
        } else if(s[0]) {
            printf("Unexpected ending: [%c]%s\n", s[0], s);
            return 0;
        }
    }
    return 1;
}




/* print a symbol and handle showing its multiplicity (apple^5 if there are 5
 * apple facts) */
static void
prinths(int *hs) {
    int i;
    for(i = 0; i < SYM_SZ; i++) {
        if(hs[i] > 1)
            printf("%s^%d ", syms[i], hs[i]);
        else if(hs[i])
            printf("%s, ", syms[i]);
    }
    printf("\n");
}

static int
match(int *a, int *b) {
    /* Note that a is the current accumulator, and b is the rule. */
    int i;
    /* Since we're only iterating up to SYM_SZ, we're only checking the left
     * hand side */
    for(i = 0; i < SYM_SZ; i++) {
        /* if symbol i is > 0 in the rule LHS and 0 in the RHS, this is NOT a
         * match. */
        /* TODO: if we kept counter of max symbols found, could stop iteration
         * early */
        if(b[i] && !a[i])
            return 0;
    }
    return 1;
}

static void
eval(void) {
    int i, r = 0, steps = 0;
    printf("AC \n"), prinths(acc);
    while(r < _rules) {
        if(match(acc, rules[r])) {
            for(i = 0; i < SYM_SZ; i++) {
                if(rules[r][i]) /* remove LHS facts from acc */
                    acc[i] -= 1;
                if(rules[r][SYM_SZ + i]) /* add RHS facts to acc */
                    acc[i] += 1;
            }
            printf("%02d \n", r), prinths(acc);
            r = 0, steps++; /* when we find a rule, we go back to the beginning (top of the deck) */
        } else
            r++;
    }
}

int
main(int argc, char *argv[]) {
    FILE *f;
    int a = 1;
    if(argc < 2)
        return !printf(
                "Vera, 6 Dec 2024.\nusage: vera input.vera\n");
    if(!(f = fopen(argv[a], "r")))
        return !printf("Source missing: %s\n", argv[a]);
    if(!fread(&src, 1, SRC_SZ, f))
        return !printf("Source empty: %s\n", argv[a]);
    if(parse(src))
        eval();
    else {
        fclose(f);
        return 1; /* return non-zero to indicate failed */
    }
    fclose(f);
    return 0;
}

