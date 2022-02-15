EXAMPLES = $(wildcard examples/*)

.PHONY: all format build_examples $(EXAMPLES)
all: format build_examples

format:
	clang-format -i $$(find . -name \*.cpp -o -name \*.ino -o -name \*.h)

build_examples: $(EXAMPLES)

$(EXAMPLES):
	arduino-cli compile --fqbn arduino:avr:nano $@
