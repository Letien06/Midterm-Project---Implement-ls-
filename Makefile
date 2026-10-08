CC ?= gcc
CPPFLAGS = -Iinclude -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS =
LDLIBS =
PYTHON = python3

SOURCES = src/main.c src/options.c src/entry.c src/listing.c src/sort.c src/format.c src/units.c src/util.c
OBJECTS = $(SOURCES:src/%.c=build/%.o)
DEPS = $(OBJECTS:.o=.d)

.PHONY: all clean test verify demo debug sanitize help
all: ls

help:
	@printf '%s\n' \
	  'make          Build ./ls' \
	  'make test     Build and run the automated test suite' \
	  'make verify   Clean build, then run all automated tests' \
	  'make demo     Run representative commands on temporary data' \
	  'make debug    Build with debug information' \
	  'make sanitize Build with AddressSanitizer and UBSan' \
	  'make clean    Remove generated build files'

ls: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

debug:
	$(MAKE) clean
	$(MAKE) CFLAGS='-std=c11 -O0 -g3 -Wall -Wextra -Wpedantic' all

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS='-std=c11 -O1 -g3 -Wall -Wextra -Wpedantic -fno-omit-frame-pointer -fsanitize=address,undefined' LDFLAGS='-fsanitize=address,undefined' all

test: all
	$(PYTHON) tests/test_ls.py ./ls

verify:
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) test

demo: all
	sh tests/demo.sh ./ls

clean:
	rm -rf build ls

-include $(DEPS)
