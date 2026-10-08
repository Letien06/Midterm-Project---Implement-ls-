#ifndef LS_ENTRY_H
#define LS_ENTRY_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>
#include <time.h>

#include "options.h"

typedef struct {
    char *name;
    char *path;
    struct stat metadata;
} Entry;

typedef struct {
    Entry *items;
    size_t length;
    size_t capacity;
} EntryList;

bool entry_load(Entry *entry, const char *path, const char *name,
                bool follow_directory_link);
void entry_destroy(Entry *entry);
/* On success the list takes ownership and clears the supplied entry. */
bool entry_list_push(EntryList *list, Entry *entry);
void entry_list_destroy(EntryList *list);
struct timespec entry_time(const Entry *entry, TimeMode time_mode);

#endif
