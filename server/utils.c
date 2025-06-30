/*
 *  utils.c
 *
 *  utility functions
*/

//==================
// include
//==================

#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

#include "utils.h"

/* decodes a string with url encoding */
void urldecode(char* dest, const char* source) {
    char a, b;
    while (*source) {
        if ((*source == '%') &&
            ((a = source[1]) && (b = source[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';

            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';

            *dest++ = 16 * a + b;
            source += 3;
        } else if (*source == '+') {
            *dest++ = ' ';
            source++;
        } else {
            *dest++ = *source++;
        }
    }
    *dest = '\0';
}


void logIP(const char* format, ...)
{
    FILE* log = fopen("logs/server-ip.log", "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm* tminfo = localtime(&now);
    char timebuf[64] = "";
    if (tminfo) strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tminfo);

    char msg[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
//    vfprintf(log, format, args);
    va_end(args);

    size_t len = strlen(msg);
    if (len > 0 && msg[len - 1] == '\n') {
        msg[len - 1] = '\0';
    }

    fprintf(log, "%s [%s]\n\n", msg, timebuf);

    fclose(log);
}




