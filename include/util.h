#ifndef LS_UTIL_H
#define LS_UTIL_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *program_name;
    bool had_error;
    bool fatal_error;
    bool wrote_output;
} Runtime;

char *path_join(const char *directory, const char *name);
bool array_reserve(void **storage, size_t *capacity, size_t needed,
                   size_t element_size);
void runtime_error(Runtime *runtime, const char *path, int error_number);
void runtime_message(Runtime *runtime, const char *path, const char *message);
bool output_ok(Runtime *runtime);

#endif
