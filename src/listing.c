#include "listing.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "entry.h"
#include "format.h"
#include "sort.h"
#include "util.h"

typedef struct {
    dev_t device;
    ino_t inode;
} DirectoryIdentity;

typedef struct {
    DirectoryIdentity *items;
    size_t length;
    size_t capacity;
} AncestorStack;

static bool visible_name(const char *name, const Options *options) {
    if (name[0] != '.') {
        return true;
    }
    if (options->show_all) {
        return true;
    }
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return false;
    }
    return options->show_almost_all;
}

static bool recursion_candidate(const Entry *entry) {
    return S_ISDIR(entry->metadata.st_mode) &&
           strcmp(entry->name, ".") != 0 && strcmp(entry->name, "..") != 0;
}

static bool ancestor_contains(const AncestorStack *stack,
                              const struct stat *metadata) {
    for (size_t index = 0; index < stack->length; ++index) {
        if (stack->items[index].device == metadata->st_dev &&
            stack->items[index].inode == metadata->st_ino) {
            return true;
        }
    }
    return false;
}

static bool ancestor_push(AncestorStack *stack, const struct stat *metadata) {
    if (!array_reserve((void **)&stack->items, &stack->capacity,
                       stack->length + 1U, sizeof(*stack->items))) {
        return false;
    }
    stack->items[stack->length].device = metadata->st_dev;
    stack->items[stack->length].inode = metadata->st_ino;
    ++stack->length;
    return true;
}

static bool sort_entries(EntryList *entries, const Options *options,
                         Runtime *runtime, const char *context) {
    if (entry_list_sort(entries, options)) {
        return true;
    }
    runtime_error(runtime, context, ENOMEM);
    return false;
}

static void begin_directory_section(const char *path, bool show_header,
                                    const Options *options, Runtime *runtime) {
    if (!show_header) {
        return;
    }
    if (runtime->wrote_output) {
        (void)putchar('\n');
    }
    (void)print_header(path, options, runtime);
}

static bool collect_directory(const char *path, const Options *options,
                              EntryList *entries, Runtime *runtime) {
    DIR *directory = opendir(path);
    if (directory == NULL) {
        runtime_error(runtime, path, errno);
        return false;
    }

    for (;;) {
        errno = 0;
        struct dirent *directory_entry = readdir(directory);
        if (directory_entry == NULL) {
            if (errno != 0) {
                runtime_error(runtime, path, errno);
            }
            break;
        }
        if (!visible_name(directory_entry->d_name, options)) {
            continue;
        }

        char *full_path = path_join(path, directory_entry->d_name);
        if (full_path == NULL) {
            int saved_error = errno == 0 ? ENOMEM : errno;
            runtime_error(runtime, path, saved_error);
            if (saved_error == ENOMEM) {
                runtime->fatal_error = true;
                break;
            }
            continue;
        }

        Entry entry;
        if (!entry_load(&entry, full_path, directory_entry->d_name, false)) {
            int saved_error = errno;
            runtime_error(runtime, full_path, saved_error);
            free(full_path);
            if (saved_error == ENOMEM) {
                runtime->fatal_error = true;
                break;
            }
            continue;
        }
        free(full_path);

        if (!entry_list_push(entries, &entry)) {
            int saved_error = errno == 0 ? ENOMEM : errno;
            runtime_error(runtime, entry.path, saved_error);
            entry_destroy(&entry);
            runtime->fatal_error = true;
            break;
        }
    }

    if (closedir(directory) != 0) {
        runtime_error(runtime, path, errno);
    }
    return !runtime->fatal_error;
}

static void list_directory(const Entry *directory, bool show_header,
                           const Options *options, Runtime *runtime,
                           AncestorStack *ancestors) {
    begin_directory_section(directory->path, show_header, options, runtime);
    if (runtime->fatal_error) {
        return;
    }

    /* Track ancestor device/inode pairs to prevent a recursive loop if a
       filesystem exposes a directory through more than one path. */
    bool pushed = false;
    if (options->recursive) {
        if (ancestor_contains(ancestors, &directory->metadata)) {
            runtime_message(runtime, directory->path, "recursive directory loop");
            return;
        }
        if (!ancestor_push(ancestors, &directory->metadata)) {
            runtime_error(runtime, directory->path, ENOMEM);
            runtime->fatal_error = true;
            return;
        }
        pushed = true;
    }

    EntryList entries = {0};
    bool collected = collect_directory(directory->path, options, &entries, runtime);
    if (collected || entries.length != 0) {
        (void)sort_entries(&entries, options, runtime, directory->path);
        (void)print_entries(&entries, options, true, runtime);
    }

    if (options->recursive && !runtime->fatal_error) {
        for (size_t index = 0; index < entries.length; ++index) {
            if (recursion_candidate(&entries.items[index])) {
                list_directory(&entries.items[index], true, options, runtime,
                               ancestors);
                if (runtime->fatal_error) {
                    break;
                }
            }
        }
    }

    entry_list_destroy(&entries);
    if (pushed) {
        --ancestors->length;
    }
}

static bool add_operand(EntryList *files, EntryList *directories,
                        const char *operand, const Options *options,
                        Runtime *runtime) {
    Entry entry;
    if (!entry_load(&entry, operand, operand, !options->directory_as_file)) {
        runtime_error(runtime, operand, errno);
        return false;
    }

    EntryList *destination = S_ISDIR(entry.metadata.st_mode) &&
                                     !options->directory_as_file
                                 ? directories
                                 : files;
    if (!entry_list_push(destination, &entry)) {
        runtime_error(runtime, operand, errno == 0 ? ENOMEM : errno);
        entry_destroy(&entry);
        runtime->fatal_error = true;
        return false;
    }
    return true;
}

void listing_run(const Options *options, int operand_count, char **operands,
                 Runtime *runtime) {
    EntryList files = {0};
    EntryList directories = {0};

    if (operand_count == 0) {
        (void)add_operand(&files, &directories, ".", options, runtime);
    } else {
        for (int index = 0; index < operand_count && !runtime->fatal_error; ++index) {
            (void)add_operand(&files, &directories, operands[index], options,
                              runtime);
        }
    }

    (void)sort_entries(&files, options, runtime, "file operands");
    (void)sort_entries(&directories, options, runtime, "directory operands");

    if (files.length != 0) {
        (void)print_entries(&files, options, false, runtime);
    }

    bool show_headers = options->recursive || directories.length > 1U ||
                        files.length != 0 || operand_count > 1;
    AncestorStack ancestors = {0};
    for (size_t index = 0; index < directories.length && !runtime->fatal_error;
         ++index) {
        list_directory(&directories.items[index], show_headers, options, runtime,
                       &ancestors);
    }

    free(ancestors.items);
    entry_list_destroy(&files);
    entry_list_destroy(&directories);
}
