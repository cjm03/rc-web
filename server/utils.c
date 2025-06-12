/*
 *  utils.c
 *
 *  utility functions
*/

#include <ctype.h>

#include "utils.h"

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
