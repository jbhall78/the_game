#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <errno.h>

#include "game.h"
#include "mkstr.h"

void
_die(const char *filename, const char *func, int line, char *fmt, ...)
{
    /* Guess we need no more than 100 bytes. */
    int n, size = 100;
    char *p;
    char *p2;
    va_list ap;

    if ((p = malloc (size)) == NULL)
        return;

    for (;;) {
        /* Try to print in the allocated space. */
        va_start(ap, fmt);
        n = vsnprintf (p, size, fmt, ap);
        va_end(ap);

        /* If that worked, return the string. */
        if (n > -1 && n < size)
            break;

        /* Else try again with more space. */
        if (n > -1)             // glibc 2.1
            size = n+1;         // precisely what is needed
        else                    // glibc 2.0
            size *= 2;          // twice the old size

        if ((p = realloc (p, size)) == NULL)
            return;
    }

#ifdef DEBUG
    if (filename != NULL && func != NULL && line != 0) {
            p2 = mkstr("[%s::%s:%d] %s", filename, func, line, p); 
    } else
#else
	p2 = strdup(p);
#endif

    //game_shutdown();

    fputs(p2, stderr);

    free(p);
    free(p2);

    exit(1);
}
