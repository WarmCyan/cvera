CC=cosmocc/bin/cosmocc

.PHONY: help
help: ## display all the make commands and their docstring
	@grep -E "^[a-zA-Z_-]+:.*?## .*$$" $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

.PHONY: clean
clean: ## remove everything in the bin folder, start fresh!
	-rm -rf bin
	-rm -rf tests/splits
	-rm -rf tests/runs
	-rm -rf tests/outs
	-rm -rf generated
	-rm -rf projects/snake_core.c

# https://justine.lol/cosmopolitan/
cosmocc: ## grab and set up the cosmopolitan toolchain (use make [...] CC=cc if you don't want to use)
	mkdir cosmocc
	cd cosmocc && wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
	cd cosmocc && unzip cosmocc.zip
	rm cosmocc/cosmocc.zip
	
.PHONY: build
build: bin/tester bin/run bin/variables bin/compile ## compile all the runnable things

# TODO: use fancy makefile vars to automate for any lists
tests/splits/parser: tests/lists/parser
	@-rm -rf tests/splits/parser
	tests/split parser

tests/splits/interpreter: tests/lists/interpreter
	@-rm -rf tests/splits/interpreter
	tests/split interpreter
	
tests/splits/variables: tests/lists/variables
	@-rm -rf tests/splits/variables
	tests/split variables

tests/splits/compiler: tests/lists/compiler
	@-rm -rf tests/splits/compiler
	tests/split compiler

bin/tester: src/parser.c src/parser.h src/tester.c src/variables_pass.h src/variables_pass.c
	@mkdir -p bin
	${CC} src/tester.c src/parser.c src/variables_pass.c -o bin/tester

bin/run: src/run.c src/interpreter.h src/interpreter.c src/parser.c src/parser.h src/variables_pass.h src/variables_pass.c
	@mkdir -p bin
	${CC} src/run.c src/parser.c src/interpreter.c src/variables_pass.c -o bin/run

bin/variables: src/variables.c src/parser.c src/parser.h src/variables_pass.h src/variables_pass.c
	@mkdir -p bin
	${CC} src/variables.c src/parser.c src/variables_pass.c -o bin/variables

bin/compile: src/compile.c src/compiler.h src/compiler.c src/parser.h src/parser.c src/interpreter.h src/interpreter.c src/variables_pass.c src/variables_pass.h
	@mkdir -p bin
	${CC} src/compile.c src/parser.c src/interpreter.c src/compiler.c src/variables_pass.c -o bin/compile

generated/salad: bin/compile
	@mkdir -p generated
	exec bin/compile tests/salad.vera > generated/salad.c
	${CC} generated/salad.c -DDEBUG -o generated/salad

generated/multiplicity2: bin/compile
	@mkdir -p generated
	exec bin/compile tests/multiplicity2.vera > generated/multiplicity2.c
	${CC} generated/multiplicity2.c -DDEBUG -o generated/multiplicity2
	
generated/vars_w_vars: bin/compile
	@mkdir -p generated
	exec bin/compile tests/vars.vera --vars > generated/vars_w_vars.c
	${CC} generated/vars_w_vars.c -DDEBUG -o generated/vars_w_vars
	
generated/vars: bin/compile
	@mkdir -p generated
	exec bin/compile tests/vars.vera > generated/vars.c
	${CC} generated/vars.c -DDEBUG -o generated/vars
	
.PHONY: test
	# exec bin/tester tests/multiplicity.vera --prules
	# cat tests/multiplicity.vera | bin/run --steps 1 --plast
test: bin/compile ## run a single example test to see parser output
	exec bin/compile tests/salad.vera

.PHONY: tests
tests: build tests/splits/parser tests/splits/interpreter tests/splits/variables tests/splits/compiler generated/salad generated/multiplicity2 generated/vars generated/vars_w_vars ## run and report on all tests
	@tests/run_tests -v



projects/snake_core.c: bin/compile projects/snake.vera
	exec bin/compile projects/snake.vera > projects/snake_core.c

bin/snake: projects/snake_core.c projects/snake.c
	${CC} projects/snake.c -o bin/snake


# .PHONY: tests-verbose
# tests-verbose: bin/tester tests/splits/parser ## run and report on all tests
# 	@tests/run_tests -v

# bin/vera: bin src/vera.c ## compile the vera interpreter
# 	mkdir -p bin
# 	cosmocc/bin/cosmocc src/vera.c -o bin/vera

# .PHONY: testgcc
# testgcc:
# 	gcc -Wall -g src/tester.c src/parser.c -o bin/tester
# 	gdb --args bin/tester tests/intro.vera

# .PHONY: run
# run: build ## run a single example that's of interest
# 	exec bin/vera tests/test4.vera

# .PHONY: debug
# debug: build ## run a single example of interest with debug stuff
# 	exec bin/vera tests/intro.vera --ftrace
