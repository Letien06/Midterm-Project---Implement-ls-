#include "entry.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void entry_reset(Entry *entry) {
    memset(entry, 0, sizeof(*entry));
}

bool entry_load(Entry *entry, const char *path, const char *name,
                bool follow_directory_link) {
    entry_reset(entry);

    if (lstat(path, &entry->metadata) != 0) {
        return false;
    }

    if (follow_directory_link && S_ISLNK(entry->metadata.st_mode)) {
        struct stat target;
        if (stat(path, &target) == 0 && S_ISDIR(target.st_mode)) {
            entry->metadata = target;
        }
    }

    entry->path = strdup(path);
    entry->name = strdup(name);
    if (entry->path == NULL || entry->name == NULL) {
        int saved_error = errno == 0 ? ENOMEM : errno;
        entry_destroy(entry);
        errno = saved_error;
        return false;
    }
    return true;
}

void entry_destroy(Entry *entry) {
    free(entry->name);
    free(entry->path);
    entry_reset(entry);
}

bool entry_list_push(EntryList *list, Entry *entry) {
    if (list->length == SIZE_MAX) {
        errno = ENOMEM;
        return false;
    }
    void *storage = list->items;
    if (!array_reserve(&storage, &list->capacity,
                       list->length + 1U, sizeof(*list->items))) {
        return false;
    }
    list->items = storage;
    /* Ownership moves into the list, so the caller cannot double-free it. */
    list->items[list->length++] = *entry;
    entry_reset(entry);
    return true;
}

void entry_list_destroy(EntryList *list) {
    for (size_t index = 0; index < list->length; ++index) {
        entry_destroy(&list->items[index]);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

struct timespec entry_time(const Entry *entry, TimeMode time_mode) {
#if defined(__APPLE__) || defined(__NetBSD__)
    switch (time_mode) {
        case TIME_ACCESSED: return entry->metadata.st_atimespec;
        case TIME_CHANGED: return entry->metadata.st_ctimespec;
        default: return entry->metadata.st_mtimespec;
    }
#else
    switch (time_mode) {
        case TIME_ACCESSED: return entry->metadata.st_atim;
        case TIME_CHANGED: return entry->metadata.st_ctim;
        default: return entry->metadata.st_mtim;
    }
#endif
}
