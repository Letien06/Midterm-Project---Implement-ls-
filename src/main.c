#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "listing.h"
#include "options.h"
#include "util.h"

static const char *program_name(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash == NULL ? path : slash + 1;
}

int main(int argc, char **argv) {
    (void)setlocale(LC_ALL, "");
    tzset();
    (void)signal(SIGPIPE, SIG_IGN);

    Options options;
    int first_operand;
    if (!options_parse(&options, argc, argv, &first_operand)) {
        return 1;
    }

    Runtime runtime = {
        .program_name = program_name(argv[0]),
        .had_error = false,
        .fatal_error = false,
        .wrote_output = false,
    };

    listing_run(&options, argc - first_operand, argv + first_operand, &runtime);
    if (fflush(stdout) == EOF || ferror(stdout)) {
        runtime_error(&runtime, "standard output", errno == 0 ? EIO : errno);
    }
    return runtime.had_error ? 1 : 0;
}
