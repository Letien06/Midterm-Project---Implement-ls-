#include "sort.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int compare_integer(off_t left, off_t right) {
    return left < right ? -1 : left > right ? 1 : 0;
}

static int compare_times(struct timespec left, struct timespec right) {
    if (left.tv_sec != right.tv_sec) {
        return left.tv_sec < right.tv_sec ? -1 : 1;
    }
    return left.tv_nsec < right.tv_nsec ? -1 : left.tv_nsec > right.tv_nsec ? 1 : 0;
}

static int compare_names(const char *left, const char *right) {
    int result = strcoll(left, right);
    if (result == 0) {
        result = strcmp(left, right);
    }
    return result < 0 ? -1 : result > 0 ? 1 : 0;
}

static int compare_entries(const Entry *left, const Entry *right,
                           const Options *options) {
    int result = 0;

    if (options->sort_mode == SORT_SIZE) {
        result = -compare_integer(left->metadata.st_size, right->metadata.st_size);
    } else if (options->sort_mode == SORT_TIME) {
        result = -compare_times(entry_time(left, options->time_mode),
                                entry_time(right, options->time_mode));
    }

    if (result == 0) {
        result = compare_names(left->name, right->name);
    }
    return options->reverse ? -result : result;
}

static void merge(Entry *items, Entry *temporary, size_t begin, size_t middle,
                  size_t end, const Options *options) {
    size_t left = begin;
    size_t right = middle;
    size_t output = begin;

    while (left < middle && right < end) {
        if (compare_entries(&items[left], &items[right], options) <= 0) {
            temporary[output++] = items[left++];
        } else {
            temporary[output++] = items[right++];
        }
    }
    while (left < middle) {
        temporary[output++] = items[left++];
    }
    while (right < end) {
        temporary[output++] = items[right++];
    }
    for (size_t index = begin; index < end; ++index) {
        items[index] = temporary[index];
    }
}

static void merge_sort(Entry *items, Entry *temporary, size_t begin, size_t end,
                       const Options *options) {
    if (end - begin < 2U) {
        return;
    }
    size_t middle = begin + (end - begin) / 2U;
    merge_sort(items, temporary, begin, middle, options);
    merge_sort(items, temporary, middle, end, options);
    merge(items, temporary, begin, middle, end, options);
}

bool entry_list_sort(EntryList *list, const Options *options) {
    if (options->no_sort || list->length < 2U) {
        return true;
    }
    if (list->length > SIZE_MAX / sizeof(*list->items)) {
        return false;
    }

    Entry *temporary = malloc(list->length * sizeof(*temporary));
    if (temporary == NULL) {
        return false;
    }
    merge_sort(list->items, temporary, 0, list->length, options);
    free(temporary);
    return true;
}
