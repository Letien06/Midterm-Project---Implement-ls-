#ifndef LS_LISTING_H
#define LS_LISTING_H

#include "options.h"
#include "util.h"

void listing_run(const Options *options, int operand_count, char **operands,
                 Runtime *runtime);

#endif
