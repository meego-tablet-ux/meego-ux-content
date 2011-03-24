#ifndef __debug_h
#define __debug_h

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "threadtest.h"

#define BACKTRACE { \
    int j, nptrs; \
    void *buffer[100]; \
    char **strings; \
    nptrs = backtrace(buffer, 100); \
    strings = backtrace_symbols(buffer, nptrs); \
    if (strings == NULL) { \
        perror("backgrace_symbols"); \
    } else { \
        for (j = 0; j < nptrs; j++) \
            printf("#%d: %s\n", j, strings[j]); \
        free(strings); \
    } }

#endif
