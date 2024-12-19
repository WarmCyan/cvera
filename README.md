# Vera C Implementation

Using https://git.sr.ht/~rabbits/vera as a starting point and just playing
around trying to understand stuff and see what I can build on top of it.

I am clearly not a C developer and have no idea what I'm doing and also don't
really understand how [vera](https://wiki.xxiivv.com/site/vera.html) (see also
[nova](https://wiki.nova-lang.net/index.php?title=Main_Page)) is supposed to
work yet, but here we are jumping off the deep end!

I've been looking for an excuse to play with
[cosmopolitan](https://github.com/jart/cosmopolitan), so I'm using that as the
compiler!

## Status

I've split out the parser code into a separate `parser.h` and `parser.c`, with
the idea that I can build separate executables for passes that all take
advantage of the same basic parsing logic.

The parser fills a rule table and symbol table struct (see
[`parser.h`](src/parser.h)) so it's not tied to a specific means of memory
management, and potentially allows for parsing into multiple different
rule/symbol tables if that becomes relevant.

I've modified it so parsing facts still enters a new row into the rules table,
just with a blank left hand side. In principle, this should mean that
"printing"/dumping out the rules table should output semantically equivalent
vera code to what was input.

I haven't yet re-implemented actual evaluation, so this codebase only parses
vera code so far.

## Running/testing

If you want to use the cosmopolitan compiler, first run
```
make cosmocc
```
(This will download a several hundred megabyte zip.) 

If you'd prefer to use a local gcc or other compiler, run the make commands
with the `CC` variable, e.g.:
```
make test CC=cc
```

To build the parser, run the tests, and see all the possible test commands
printed out, run: 
```
make tests
```

Each test line is the command that was run for that test, to run manually to see
the output, you can do something like: 
```
bin/tester tests/intro.vera --psymbols
```
Which prints out the list of symbols the parser parsed, or:

```
bin/tester tests/intro.vera --prules
```
Which prints out the rules table/should be semantically equivalent vera code.
