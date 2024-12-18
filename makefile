.PHONY: help
help: ## display all the make commands and their docstring
	@grep -E "^[a-zA-Z_-]+:.*?## .*$$" $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

# https://justine.lol/cosmopolitan/
cosmocc: ## grab and set up the cosmopolitan toolchain
	mkdir cosmocc
	cd cosmocc && wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
	cd cosmocc && unzip cosmocc.zip

bin:
	mkdir bin


# TODO: use fancy makefile vars to automate for any lists
tests/splits/parser: tests/lists/parser
	@-rm -rf tests/splits/parser
	tests/split parser

.PHONY: build
build: bin/vera ## run all compiliation steps

bin/vera: bin cosmocc src/vera.c ## compile the vera interpreter
	mkdir -p bin
	cosmocc/bin/cosmocc src/vera.c -o bin/vera

bin/tester: bin cosmocc src/parser.c src/parser.h src/tester.c
	cosmocc/bin/cosmocc src/tester.c src/parser.c -o bin/tester

.PHONY: test
test: bin/tester
	exec bin/tester tests/intro.vera --prules

.PHONY: testgcc
testgcc:
	gcc -Wall -g src/tester.c src/parser.c -o bin/tester
	gdb --args bin/tester tests/intro.vera

.PHONY: clean
clean: ## remove everything in the bin folder, start fresh!
	rm -rf bin
	rm -rf tests/splits
	rm -rf tests/runs
	rm -rf tests/outs


.PHONY: run
run: build ## run a single example that's of interest
	exec bin/vera tests/test4.vera

.PHONY: debug
debug: build ## run a single example of interest with debug stuff
	exec bin/vera tests/intro.vera --ftrace

.PHONY: tests
tests: bin/tester tests/splits/parser ## run and report on all tests
	@tests/run_tests -v

# .PHONY: tests-verbose
# tests-verbose: bin/tester tests/splits/parser ## run and report on all tests
# 	@tests/run_tests -v
