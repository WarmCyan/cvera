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
#define DIC_SZ 0x8000 /* TODO: ? */
#define SYM_SZ 0x100  /* maximum length of a symbol...or is it the maximum number of unique symbols? */
#define RUL_SZ 0x80   /* maximum number of rules we can handle */

static char spacer_glyph, src[SRC_SZ];
/* ahhh the secondary pointer definitions are because arrays aren't technically
 * pointers, at least wrt to incrementing, see https://stackoverflow.com/questions/4607128/in-c-are-arrays-pointers-or-used-as-pointers
 * (incrementing a pointer moves by 4 bytes, I assume incrementing an array does
 * not? or maybe you just want a pointer separate from the array to be able to
 * iterate/index into it without losing the pointer to the beginning of the
 * array? */
static char dict[DIC_SZ], *_dict = dict;
static char *syms[SYM_SZ], **_syms = syms;
static int acc[SYM_SZ], rules[RUL_SZ][SYM_SZ * 2], _rules = 0;

/* iterate through source until we find a non-whitespace character */
static char *
walk_whitespace(char *s) {
    /* TODO: I added this s - src < SRC_SZ, because without I kept getting
     * segfaults after removing the check for line feed in walk_symbol, but now
     * we're getting different registers at the end than original.c */
	while(s[0] && s[0] <= 0x20 && s - src < SRC_SZ)
		s++;
	return s;
}

/* ...go to the end of src? */
/* hmmm no this is just finding the next null term? */
static char *
scap(char *s) {
	while(*s) s++;
	return s;
}

static void
trim(char **s) {
	char *cap = scap(*s) - 1;
	while(*cap) {
		if(*cap > 0x20) break;
		*cap = 0; /* add null terminators at the end of the symbol until we find a ? */
		cap--;
	}
}

/* check if the two passed symbols are the same */
static int
symbol_compare(char *a, char *b) {
	while(*a && *b) {
		if(*a == ',' && !*b) break;
		if(*a != *b) break;
		if(*a == ' ') a = walk_whitespace(a);
		if(*b == ' ') b = walk_whitespace(b);
		a++, b++;
	}
    /* two symbols are the same if we've made it this far and a is indeed at the
     * end of a symbol? (comma, spacer, or whitespace) */
    /* OH. I guess we're negating *b because if we iterated to the end of it,
     * it's gonna be a null term? so !*b should be non-null (thus true) and a
     * should be the end of a symbol in some way */
	return !*b && (*a == ',' || *a == spacer_glyph || *a <= 0x20);
}

static int
find_symbol_index(char *s) {
	int i, len = _syms - &syms[0];
	for(i = 0; i < len; i++)
		if(symbol_compare(s, syms[i]))
			return i;
	return -1;
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
		while(s && s[0] != spacer_glyph && s[0] != ',' /*&& s[0] != 0xa*/ && s[0] != spacer_glyph)
			*s++;
		return s;
	}
	*_syms = _dict;
	while(s[0] && s[0] != spacer_glyph && s[0] != ',' /*&& s[0] != 0xa*/ && s[0] != spacer_glyph) {
		*_dict++ = *s++;
		if(*s == ' ')
			s = walk_whitespace(s), *_dict++ = ' ';
	}
	trim(_syms);
	*_dict++ = 0, *id = _syms - &syms[0], _syms++;
	return s;
}

/* Note that this returns next point within source code that isn't a
 * whitespace */
static char *
walk_rule(char *s) {
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

print_all_rules() {
    printf("-------\n");
    int i;
    for (i = 0; i < RUL_SZ; i++) {
        if (rules[i]) {
            printf("RUL %d:%s\n", i, rules[i]);
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
	int i;
	for(i = 0; i < SYM_SZ; i++) {
		if(b[i] && !a[i])
			return 0;
	}
	return 1;
}

static void
eval(void) {
	int i, r = 0, steps = 0;
	printf("AC "), prinths(acc);
	while(r < _rules) {
		if(match(acc, rules[r])) {
			for(i = 0; i < SYM_SZ; i++) {
				if(rules[r][i])
					acc[i] -= 1;
				if(rules[r][SYM_SZ + i])
					acc[i] += 1;
			}
			printf("%02d \n", r), prinths(acc);
			r = 0, steps++;
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

