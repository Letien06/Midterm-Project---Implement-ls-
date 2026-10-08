#ifndef LS_OPTIONS_H
#define LS_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum { SORT_NAME, SORT_SIZE, SORT_TIME } SortMode;
typedef enum { TIME_MODIFIED, TIME_CHANGED, TIME_ACCESSED } TimeMode;
typedef enum { FORMAT_SHORT, FORMAT_LONG, FORMAT_NUMERIC } OutputMode;
typedef enum { SIZE_BLOCKS, SIZE_KIBIBYTES, SIZE_HUMAN } SizeMode;

typedef struct {
    bool show_all;
    bool show_almost_all;
    bool classify;
    bool no_sort;
    bool print_inode;
    bool print_blocks;
    bool reverse;
    bool recursive;
    bool directory_as_file;
    bool output_is_terminal;
    bool replace_nonprintable;
    SortMode sort_mode;
    TimeMode time_mode;
    OutputMode output_mode;
    SizeMode size_mode;
    uintmax_t block_size;
} Options;

/* Parse left to right; only explicitly documented pairs reset each other. */
bool options_parse(Options *options, int argc, char **argv, int *first_operand);

#endif
