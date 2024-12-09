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

.PHONY: buildoriginal
buildoriginal: bin/original ## compile the original vera.c file, useful for testing

bin/original: cosmocc src/original.c ## compile original vera.c file
	mkdir -p bin
	cosmocc/bin/cosmocc src/original.c -o bin/original
	

.PHONY: runall
runall: build buildoriginal ## run each test
	exec bin/vera tests/intro.vera
	@echo "--------------------------------------------------"
	exec bin/original tests/intro.vera
	@echo "=================================================="
	exec bin/vera tests/salad.vera
	@echo "--------------------------------------------------"
	exec bin/original tests/salad.vera
	@echo "=================================================="
	exec bin/vera tests/test1.vera
	@echo "=================================================="
	exec bin/vera tests/test2.vera

.PHONY: clean
clean: ## remove everything in the bin folder, start fresh!
	rm -rf bin


.PHONY: run
run: build ## run a single example that's of interest
	exec bin/vera tests/intro.vera


.PHONY: debug
debug: build ## run a single example of interest with debug stuff
	exec bin/vera tests/intro.vera --ftrace
