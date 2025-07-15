/*
 *  utils.c
 *
 *  utility functions
*/

//==================
// include
//==================

#include <ctype.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

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

void hexStringToBytes(const char* hexstr, unsigned char* buffer, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        sscanf(hexstr + 2 * i, "%2hhx", &buffer[i]);
    }
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
    va_end(args);

    size_t len = strlen(msg);
    if (len > 0 && msg[len - 1] == '\n') {
        msg[len - 1] = '\0';
    }

    fprintf(log, "%s [%s]\n\n", msg, timebuf);

    fclose(log);
}


SSL_CTX* initSSLCTX(void)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD* method = TLS_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}


void loadCerts(SSL_CTX* ctx, const char* certFile, const char* keyFile)
{
    if (SSL_CTX_use_certificate_file(ctx, certFile, SSL_FILETYPE_PEM) <= 0 || 
        SSL_CTX_use_PrivateKey_file(ctx, keyFile, SSL_FILETYPE_PEM) <= 0 || 
        !SSL_CTX_check_private_key(ctx)) {
        ERR_print_errors_fp(stderr);
        abort();
    }
}




