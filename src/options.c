#include "options.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "units.h"

static void options_init(Options *options) {
    memset(options, 0, sizeof(*options));
    options->show_almost_all = geteuid() == 0;
    options->output_is_terminal = isatty(STDOUT_FILENO) != 0;
    options->replace_nonprintable = options->output_is_terminal;
    options->sort_mode = SORT_NAME;
    options->time_mode = TIME_MODIFIED;
    options->output_mode = FORMAT_SHORT;
    options->size_mode = SIZE_BLOCKS;
    options->block_size = parse_block_size(getenv("BLOCKSIZE"));
}

bool options_parse(Options *options, int argc, char **argv, int *first_operand) {
    /* State is updated as options are read so the documented rightmost option
       wins for each mutually exclusive pair. */
    options_init(options);
    opterr = 0;
    optind = 1;

    int flag;
    while ((flag = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (flag) {
            case 'A':
                options->show_almost_all = true;
                options->show_all = false;
                break;
            case 'a':
                options->show_all = true;
                options->show_almost_all = false;
                break;
            case 'c':
                options->time_mode = TIME_CHANGED;
                break;
            case 'd':
                options->directory_as_file = true;
                options->recursive = false;
                break;
            case 'F':
                options->classify = true;
                break;
            case 'f':
                options->no_sort = true;
                break;
            case 'h':
                options->size_mode = SIZE_HUMAN;
                break;
            case 'i':
                options->print_inode = true;
                break;
            case 'k':
                options->size_mode = SIZE_KIBIBYTES;
                break;
            case 'l':
                options->output_mode = FORMAT_LONG;
                break;
            case 'n':
                options->output_mode = FORMAT_NUMERIC;
                break;
            case 'q':
                options->replace_nonprintable = true;
                break;
            case 'R':
                options->recursive = true;
                options->directory_as_file = false;
                break;
            case 'r':
                options->reverse = true;
                break;
            case 'S':
                options->sort_mode = SORT_SIZE;
                break;
            case 's':
                options->print_blocks = true;
                break;
            case 't':
                options->sort_mode = SORT_TIME;
                break;
            case 'u':
                options->time_mode = TIME_ACCESSED;
                break;
            case 'w':
                options->replace_nonprintable = false;
                break;
            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], optopt);
                fprintf(stderr, "usage: %s [-AacdFfhiklnqRrSstuw] [file ...]\n", argv[0]);
                return false;
        }
    }

    *first_operand = optind;
    return true;
}
