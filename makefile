.PHONY: help
help: ## display all the make commands and their docstring
	@grep -E "^[a-zA-Z_-]+:.*?## .*$$" $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

# https://justine.lol/cosmopolitan/
cosmocc: ## grab and set up the cosmopolitan toolchain
	mkdir cosmocc
	cd cosmocc && wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
	cd cosmocc && unzip cosmocc.zip

.PHONY: build
build: bin/vera ## run all compiliation steps

bin/vera: cosmocc src/vera.c ## compile the vera interpreter
	mkdir -p bin
	cosmocc/bin/cosmocc src/vera.c -o bin/vera


bin/tester: cosmocc src/parser.c src/parser.h src/tester.c
	cosmocc/bin/cosmocc -Wall -g src/tester.c src/parser.c -o bin/tester

.PHONY: test
test: bin/tester
	exec bin/tester tests/intro.vera


.PHONY: clean
clean: ## remove everything in the bin folder, start fresh!
	rm -rf bin


.PHONY: run
run: build ## run a single example that's of interest
	exec bin/vera tests/test4.vera

.PHONY: debug
debug: build ## run a single example of interest with debug stuff
	exec bin/vera tests/intro.vera --ftrace

.PHONY: tests
tests: build ## run and report on all tests
	@tests/run vera intro
	@tests/run vera salad
	@tests/run vera test1
	@tests/run vera test2
	@tests/run vera test3
	@tests/run vera test4

.PHONY: testoriginal
testoriginal: buildoriginal ## run and report on all tests for original.c
	@tests/run original intro
	@tests/run original salad
	@tests/run original test1
	@tests/run original test2
