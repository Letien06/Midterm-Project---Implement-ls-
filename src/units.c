#include "units.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static bool consume_suffix(const char *text, uintmax_t *factor) {
    if (*text == '\0') {
        *factor = 1;
        return true;
    }

    switch (toupper((unsigned char)*text++)) {
        case 'K': *factor = UINTMAX_C(1024); break;
        case 'M': *factor = UINTMAX_C(1024) * UINTMAX_C(1024); break;
        case 'G': *factor = UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024); break;
        case 'T': *factor = UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024); break;
        case 'P': *factor = UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024); break;
        case 'E': *factor = UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024) * UINTMAX_C(1024); break;
        default: return false;
    }

    if (*text == 'i' || *text == 'I') {
        ++text;
    }
    if (*text == 'b' || *text == 'B') {
        ++text;
    }
    while (isspace((unsigned char)*text)) {
        ++text;
    }
    return *text == '\0';
}

uintmax_t parse_block_size(const char *value) {
    const uintmax_t fallback = 512;
    if (value == NULL) {
        return fallback;
    }

    while (isspace((unsigned char)*value)) {
        ++value;
    }
    if (*value == '\0') {
        return fallback;
    }

    errno = 0;
    /* BLOCKSIZE is user input; invalid, zero, and overflowing values use the
       manual's ordinary 512-byte fallback rather than becoming a divisor. */
    char *end = NULL;
    uintmax_t amount = strtoumax(value, &end, 10);
    if (errno == ERANGE || end == value || amount == 0) {
        return fallback;
    }

    uintmax_t factor;
    if (!consume_suffix(end, &factor) || amount > UINTMAX_MAX / factor) {
        return fallback;
    }
    return amount * factor;
}

bool allocated_bytes(intmax_t blocks, uintmax_t *bytes) {
    if (blocks <= 0) {
        *bytes = 0;
        return true;
    }
    if ((uintmax_t)blocks > UINTMAX_MAX / UINTMAX_C(512)) {
        return false;
    }
    *bytes = (uintmax_t)blocks * UINTMAX_C(512);
    return true;
}

bool add_size(uintmax_t left, uintmax_t right, uintmax_t *result) {
    if (left > UINTMAX_MAX - right) {
        return false;
    }
    *result = left + right;
    return true;
}

void format_human_size(uintmax_t bytes, char *buffer, size_t capacity) {
    static const char suffixes[] = "BKMGTPE";
    long double amount = (long double)bytes;
    size_t suffix = 0;

    while (amount >= 1024.0L && suffix + 1U < sizeof(suffixes) - 1U) {
        amount /= 1024.0L;
        ++suffix;
    }

    if (suffix == 0) {
        (void)snprintf(buffer, capacity, "%juB", bytes);
    } else if (amount < 10.0L) {
        (void)snprintf(buffer, capacity, "%.1Lf%c", amount, suffixes[suffix]);
    } else {
        (void)snprintf(buffer, capacity, "%.0Lf%c", amount, suffixes[suffix]);
    }
}

void format_block_count(uintmax_t bytes, const Options *options,
                        char *buffer, size_t capacity) {
    if (options->size_mode == SIZE_HUMAN) {
        format_human_size(bytes, buffer, capacity);
        return;
    }

    uintmax_t unit = options->size_mode == SIZE_KIBIBYTES ? 1024 : options->block_size;
    uintmax_t count = bytes / unit + (bytes % unit != 0);
    (void)snprintf(buffer, capacity, "%ju", count);
}
