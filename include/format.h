#ifndef LS_FORMAT_H
#define LS_FORMAT_H

#include "entry.h"
#include "options.h"
#include "util.h"

void print_name(const char *name, const Options *options);
bool print_header(const char *path, const Options *options, Runtime *runtime);
bool print_entries(const EntryList *entries, const Options *options,
                    bool directory_listing, Runtime *runtime);

#endif
