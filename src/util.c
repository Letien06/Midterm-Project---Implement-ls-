#include "util.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *path_join(const char *directory, const char *name) {
    size_t directory_length = strlen(directory);
    size_t name_length = strlen(name);
    bool needs_separator = directory_length != 0 &&
                           directory[directory_length - 1] != '/';
    size_t extra = needs_separator ? 2U : 1U;

    if (name_length > SIZE_MAX - extra ||
        directory_length > SIZE_MAX - name_length - extra) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    char *joined = malloc(directory_length + name_length + extra);
    if (joined == NULL) {
        return NULL;
    }

    memcpy(joined, directory, directory_length);
    size_t offset = directory_length;
    if (needs_separator) {
        joined[offset++] = '/';
    }
    memcpy(joined + offset, name, name_length + 1U);
    return joined;
}

bool array_reserve(void **storage, size_t *capacity, size_t needed,
                   size_t element_size) {
    if (needed <= *capacity) {
        return true;
    }

    size_t new_capacity = *capacity == 0 ? 16U : *capacity;
    while (new_capacity < needed) {
        if (new_capacity > SIZE_MAX / 2U) {
            new_capacity = needed;
            break;
        }
        new_capacity *= 2U;
    }

    if (element_size != 0 && new_capacity > SIZE_MAX / element_size) {
        errno = ENOMEM;
        return false;
    }

    void *resized = realloc(*storage, new_capacity * element_size);
    if (resized == NULL) {
        return false;
    }

    *storage = resized;
    *capacity = new_capacity;
    return true;
}

void runtime_error(Runtime *runtime, const char *path, int error_number) {
    const char *name = runtime->program_name == NULL ? "ls" : runtime->program_name;
    fprintf(stderr, "%s: %s: %s\n", name, path, strerror(error_number));
    runtime->had_error = true;
}

void runtime_message(Runtime *runtime, const char *path, const char *message) {
    const char *name = runtime->program_name == NULL ? "ls" : runtime->program_name;
    fprintf(stderr, "%s: %s: %s\n", name, path, message);
    runtime->had_error = true;
}

bool output_ok(Runtime *runtime) {
    if (ferror(stdout)) {
        runtime->had_error = true;
        runtime->fatal_error = true;
        return false;
    }
    return true;
}
