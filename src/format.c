#include "format.h"

#include <errno.h>
#include <grp.h>
#include <inttypes.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#if defined(__linux__)
#include <sys/sysmacros.h>
#endif

#include "units.h"

typedef struct {
    size_t inode;
    size_t blocks;
    size_t links;
    size_t owner;
    size_t group;
    size_t size;
} FieldWidths;

static char file_type(mode_t mode) {
#if defined(S_IFARCH1)
    if ((mode & S_IFMT) == S_IFARCH1) return 'a';
#endif
#if defined(S_IFARCH2)
    if ((mode & S_IFMT) == S_IFARCH2) return 'A';
#endif
    if (S_ISBLK(mode)) return 'b';
    if (S_ISCHR(mode)) return 'c';
    if (S_ISDIR(mode)) return 'd';
    if (S_ISLNK(mode)) return 'l';
    if (S_ISSOCK(mode)) return 's';
    if (S_ISFIFO(mode)) return 'p';
#if defined(S_ISWHT)
    if (S_ISWHT(mode)) return 'w';
#endif
    if (S_ISREG(mode)) return '-';
    return '?';
}

static void format_mode(mode_t mode, char text[11]) {
    text[0] = file_type(mode);
    text[1] = (mode & S_IRUSR) ? 'r' : '-';
    text[2] = (mode & S_IWUSR) ? 'w' : '-';
    text[3] = (mode & S_ISUID) ? ((mode & S_IXUSR) ? 's' : 'S')
                              : ((mode & S_IXUSR) ? 'x' : '-');
    text[4] = (mode & S_IRGRP) ? 'r' : '-';
    text[5] = (mode & S_IWGRP) ? 'w' : '-';
    text[6] = (mode & S_ISGID) ? ((mode & S_IXGRP) ? 's' : 'S')
                              : ((mode & S_IXGRP) ? 'x' : '-');
    text[7] = (mode & S_IROTH) ? 'r' : '-';
    text[8] = (mode & S_IWOTH) ? 'w' : '-';
#if defined(S_ISVTX)
    text[9] = (mode & S_ISVTX) ? ((mode & S_IXOTH) ? 't' : 'T')
                              : ((mode & S_IXOTH) ? 'x' : '-');
#else
    text[9] = (mode & S_IXOTH) ? 'x' : '-';
#endif
    text[10] = '\0';
}

static void owner_text(const Entry *entry, const Options *options,
                       char *buffer, size_t capacity) {
    if (options->output_mode != FORMAT_NUMERIC) {
        struct passwd *owner = getpwuid(entry->metadata.st_uid);
        if (owner != NULL) {
            (void)snprintf(buffer, capacity, "%s", owner->pw_name);
            return;
        }
    }
    (void)snprintf(buffer, capacity, "%ju", (uintmax_t)entry->metadata.st_uid);
}

static void group_text(const Entry *entry, const Options *options,
                       char *buffer, size_t capacity) {
    if (options->output_mode != FORMAT_NUMERIC) {
        struct group *group = getgrgid(entry->metadata.st_gid);
        if (group != NULL) {
            (void)snprintf(buffer, capacity, "%s", group->gr_name);
            return;
        }
    }
    (void)snprintf(buffer, capacity, "%ju", (uintmax_t)entry->metadata.st_gid);
}

static void size_text(const Entry *entry, const Options *options,
                      char *buffer, size_t capacity) {
    if (S_ISCHR(entry->metadata.st_mode) || S_ISBLK(entry->metadata.st_mode)) {
        (void)snprintf(buffer, capacity, "%ju, %ju",
                       (uintmax_t)major(entry->metadata.st_rdev),
                       (uintmax_t)minor(entry->metadata.st_rdev));
    } else if (options->size_mode == SIZE_HUMAN) {
        uintmax_t size = entry->metadata.st_size < 0
                             ? 0
                             : (uintmax_t)entry->metadata.st_size;
        format_human_size(size, buffer, capacity);
    } else {
        (void)snprintf(buffer, capacity, "%jd", (intmax_t)entry->metadata.st_size);
    }
}

static bool block_text(const Entry *entry, const Options *options,
                       char *buffer, size_t capacity) {
    uintmax_t bytes;
    if (!allocated_bytes((intmax_t)entry->metadata.st_blocks, &bytes)) {
        return false;
    }
    format_block_count(bytes, options, buffer, capacity);
    return true;
}

static void date_text(const Entry *entry, const Options *options,
                      char *buffer, size_t capacity) {
    struct timespec timestamp = entry_time(entry, options->time_mode);
    struct tm broken_down;
    if (localtime_r(&timestamp.tv_sec, &broken_down) == NULL ||
        strftime(buffer, capacity, "%b %e %H:%M", &broken_down) == 0) {
        (void)snprintf(buffer, capacity, "??? ?? ??:??");
    }
}

static void update_width(size_t *width, const char *text) {
    size_t length = strlen(text);
    if (length > *width) {
        *width = length;
    }
}

static bool collect_widths(const EntryList *entries, const Options *options,
                           FieldWidths *widths, Runtime *runtime) {
    memset(widths, 0, sizeof(*widths));
    char text[128];

    for (size_t index = 0; index < entries->length; ++index) {
        const Entry *entry = &entries->items[index];
        if (options->print_inode) {
            (void)snprintf(text, sizeof(text), "%ju",
                           (uintmax_t)entry->metadata.st_ino);
            update_width(&widths->inode, text);
        }
        if (options->print_blocks) {
            if (!block_text(entry, options, text, sizeof(text))) {
                runtime_message(runtime, entry->path, "allocated size is too large");
                return false;
            }
            update_width(&widths->blocks, text);
        }
        if (options->output_mode != FORMAT_SHORT) {
            (void)snprintf(text, sizeof(text), "%ju",
                           (uintmax_t)entry->metadata.st_nlink);
            update_width(&widths->links, text);
            owner_text(entry, options, text, sizeof(text));
            update_width(&widths->owner, text);
            group_text(entry, options, text, sizeof(text));
            update_width(&widths->group, text);
            size_text(entry, options, text, sizeof(text));
            update_width(&widths->size, text);
        }
    }
    return true;
}

static void print_padding(size_t count) {
    while (count-- != 0) {
        (void)putchar(' ');
    }
}

static void print_right_aligned(const char *text, size_t width) {
    size_t length = strlen(text);
    if (length < width) {
        print_padding(width - length);
    }
    (void)fputs(text, stdout);
}

static void print_left_aligned(const char *text, size_t width) {
    size_t length = strlen(text);
    (void)fputs(text, stdout);
    if (length < width) {
        print_padding(width - length);
    }
}

void print_name(const char *name, const Options *options) {
    if (!options->replace_nonprintable) {
        (void)fputs(name, stdout);
        return;
    }

    mbstate_t state;
    memset(&state, 0, sizeof(state));
    const char *cursor = name;
    size_t remaining = strlen(name);

    while (remaining != 0) {
        wchar_t wide_character;
        size_t consumed = mbrtowc(&wide_character, cursor, remaining, &state);
        if (consumed == (size_t)-1 || consumed == (size_t)-2) {
            (void)putchar('?');
            ++cursor;
            --remaining;
            memset(&state, 0, sizeof(state));
        } else if (consumed == 0) {
            break;
        } else {
            if (iswprint(wide_character)) {
                (void)fwrite(cursor, 1, consumed, stdout);
            } else {
                (void)putchar('?');
            }
            cursor += consumed;
            remaining -= consumed;
        }
    }
}

static char classification_suffix(mode_t mode) {
    if (S_ISDIR(mode)) return '/';
    if (S_ISLNK(mode)) return '@';
#if defined(S_ISWHT)
    if (S_ISWHT(mode)) return '%';
#endif
    if (S_ISSOCK(mode)) return '=';
    if (S_ISFIFO(mode)) return '|';
    if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return '*';
    return '\0';
}

static char *read_link_target(const Entry *entry) {
    /* st_size is only a hint and can be stale; grow until readlink fits and
       leave room for the terminating NUL ourselves. */
    size_t capacity = 128U;

    for (;;) {
        char *buffer = malloc(capacity + 1U);
        if (buffer == NULL) {
            errno = ENOMEM;
            return NULL;
        }
        ssize_t length = readlink(entry->path, buffer, capacity);
        if (length < 0) {
            int saved_error = errno;
            free(buffer);
            errno = saved_error;
            return NULL;
        }
        if ((size_t)length < capacity) {
            buffer[length] = '\0';
            return buffer;
        }
        free(buffer);
        if (capacity > (SIZE_MAX - 1U) / 2U) {
            errno = ENAMETOOLONG;
            return NULL;
        }
        capacity *= 2U;
    }
}

static bool print_total(const EntryList *entries, const Options *options,
                        Runtime *runtime) {
    uintmax_t total_bytes = 0;
    for (size_t index = 0;
         index < entries->length && !runtime->fatal_error;
         ++index) {
        uintmax_t bytes;
        uintmax_t new_total;
        if (!allocated_bytes((intmax_t)entries->items[index].metadata.st_blocks,
                             &bytes) ||
            !add_size(total_bytes, bytes, &new_total)) {
            runtime_message(runtime, entries->items[index].path,
                            "total allocated size is too large");
            return false;
        }
        total_bytes = new_total;
    }

    Options total_options = *options;
    if (!options->print_blocks && total_options.size_mode == SIZE_KIBIBYTES) {
        total_options.size_mode = SIZE_BLOCKS;
    }

    char text[128];
    format_block_count(total_bytes, &total_options, text, sizeof(text));
    (void)printf("total %s\n", text);
    runtime->wrote_output = true;
    return output_ok(runtime);
}

static bool print_entry(const Entry *entry, const Options *options,
                        const FieldWidths *widths, Runtime *runtime) {
    char text[128];

    if (options->print_inode) {
        (void)snprintf(text, sizeof(text), "%ju", (uintmax_t)entry->metadata.st_ino);
        print_right_aligned(text, widths->inode);
        (void)putchar(' ');
    }
    if (options->print_blocks) {
        if (!block_text(entry, options, text, sizeof(text))) {
            runtime_message(runtime, entry->path, "allocated size is too large");
            return false;
        }
        print_right_aligned(text, widths->blocks);
        (void)putchar(' ');
    }
    if (options->output_mode != FORMAT_SHORT) {
        char mode[11];
        format_mode(entry->metadata.st_mode, mode);
        (void)printf("%s ", mode);

        (void)snprintf(text, sizeof(text), "%ju", (uintmax_t)entry->metadata.st_nlink);
        print_right_aligned(text, widths->links);
        (void)putchar(' ');

        owner_text(entry, options, text, sizeof(text));
        print_left_aligned(text, widths->owner);
        (void)putchar(' ');

        group_text(entry, options, text, sizeof(text));
        print_left_aligned(text, widths->group);
        (void)putchar(' ');

        size_text(entry, options, text, sizeof(text));
        print_right_aligned(text, widths->size);
        (void)putchar(' ');

        date_text(entry, options, text, sizeof(text));
        (void)printf("%s ", text);
    }

    print_name(entry->name, options);
    if (options->classify) {
        char suffix = classification_suffix(entry->metadata.st_mode);
        if (suffix != '\0') {
            (void)putchar(suffix);
        }
    }

    if (options->output_mode != FORMAT_SHORT && S_ISLNK(entry->metadata.st_mode)) {
        char *target = read_link_target(entry);
        if (target == NULL) {
            runtime_error(runtime, entry->path, errno);
        } else {
            (void)fputs(" -> ", stdout);
            print_name(target, options);
            free(target);
        }
    }

    (void)putchar('\n');
    runtime->wrote_output = true;
    return output_ok(runtime);
}

bool print_header(const char *path, const Options *options, Runtime *runtime) {
    print_name(path, options);
    (void)fputs(":\n", stdout);
    runtime->wrote_output = true;
    return output_ok(runtime);
}

bool print_entries(const EntryList *entries, const Options *options,
                   bool directory_listing, Runtime *runtime) {
    bool success = true;
    if (directory_listing &&
        (options->output_mode != FORMAT_SHORT ||
         (options->print_blocks && options->output_is_terminal))) {
        success = print_total(entries, options, runtime) && success;
    }

    FieldWidths widths;
    if (!collect_widths(entries, options, &widths, runtime)) {
        return false;
    }

    for (size_t index = 0;
         index < entries->length && !runtime->fatal_error;
         ++index) {
        if (!print_entry(&entries->items[index], options, &widths, runtime)) {
            success = false;
        }
    }
    return success;
}
