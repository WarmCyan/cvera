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

the interpreter (`bin/run`) can either execute on a passed file containing vera
source code, or it can be piped in via stdin.

An implicit constants pass is handled in the parser for any `x:50` syntax,
disable by running with `--no-implicit-constants`

I've implemented half of a variables annotation pass, it adds the rules for the
destructive movement form `a -> b`, I still need to add `a = b`.

A basic vera to C compiler is available which transpiles input vera code into C.
The output code can optionally be compiled with the `-DDEBUG` flag to make a
runnable that prints out the end state accumulator.

## Bin files

Use `make build` to produce these:

* `bin/tester` - this is effectively a debugger for the parser, pipe or load any
  vera code with this and use `--psymbols` to print out the symbols table, and
  `--prules` to print out the rules list (should be semantically equivalent code
  to what was input) from the parser.
* `bin/run` - a vera interpreter, pipe or load vera code and it will run through
  it step by step. Use `--plast` to only print out the final symbols in the
  accumulator, or don't to see the bag at each step. Specify `--steps NUM` to
  set a maximum number of steps to evaluate.
* `bin/variables` - a variables compiler pass, turns a `|#| variables, ...`
  annotation into the appropriate rule set and prints it to stdout. This means
  you can use it as part of a piped chain into the interpreter, e.g.:
* `bin/compile` - a vera to c compiler, turns stdin vera code or passed vera
  source file into a string of C code. Compile the results with `gcc` or
  similar. (run `make generated/salad` for an example)

```bash
cat tests/vars.vera | bin/variables | bin/run
```

## Passes

* Whitespace trimming is currently forced, unconfigurable, and first in
  any/every parsing chain
* Constants (`x:50` syntax) are implicitly parsed, but this is optional,
  `bin/tester` and `bin/run` both take a `--no-implicit-constants` flag to
  disable
* A variables (`|#| variables, a, b`, `a -> b`) pass can optionally be run,
  either by directly transforming vera code via the discrete `bin/variables`
  command (see example snippet above under bin files) or by specifying the
  `--vars` flag for either `bin/tester` or `bin/run`. (Note that only `a -> b`
  syntax rules have been added, not `a = b`, yet)

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
