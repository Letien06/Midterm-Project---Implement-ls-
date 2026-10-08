#ifndef LS_UNITS_H
#define LS_UNITS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "options.h"

uintmax_t parse_block_size(const char *value);
bool allocated_bytes(intmax_t blocks, uintmax_t *bytes);
bool add_size(uintmax_t left, uintmax_t right, uintmax_t *result);
void format_human_size(uintmax_t bytes, char *buffer, size_t capacity);
void format_block_count(uintmax_t bytes, const Options *options,
                        char *buffer, size_t capacity);

#endif
