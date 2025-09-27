/*
 *  utils.c
 *
 *  utility functions
*/

//==================
// include
//==================

// #define _DEFAULT_SOURCE
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
// #include "video.h"

#define BUFFER_SIZE 8192

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
    FILE* log = fopen("logs/serverip.log", "a");
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
    // OpenSSL_add_all_algorithms();
    OpenSSL_add_ssl_algorithms();
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

int findHeaderEnd(const char* buf, int len)
{
    for (int i = 0; i < len - 3; ++i) {
        if (buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' && buf[i + 3] == '\n' ) {
            return i + 4;
        }
    }
    return -1;
}

int extractContentLength(const char* headers)
{
    const char* cl = strstr(headers, "Content-Length:");
    if (!cl) return 0;
    cl += strlen("Content-Length:");
    while (*cl == ' ' || *cl == '\t') cl++;
    return atoi(cl);
}

char* readFullRequest(SSL* ssl, int* outlen)
{
    char buf[BUFFER_SIZE];
    int totalread = 0;
    int headerend = -1;

    while (totalread < BUFFER_SIZE) {
        int bytes = SSL_read(ssl, buf + totalread, BUFFER_SIZE - totalread);
        if (bytes <= 0) break;
        totalread += bytes;

        headerend = findHeaderEnd(buf, totalread);
        if (headerend != -1) break;
    }
    if (headerend == -1) {
        *outlen = totalread;
        return strndup(buf, totalread);
    }

    char headers[headerend + 1];
    memcpy(headers, buf, headerend);
    headers[headerend] = '\0';
    int contentlength = extractContentLength(headers);

    int bodybytes = totalread - headerend;
    int toread = contentlength - bodybytes;

    char* request = malloc(headerend + contentlength + 1);
    if (!request) return NULL;
    memcpy(request, buf, totalread);

    int offset = totalread;
    while (toread > 0) { // && offset < headerend + contentlength) {
        int bytes = SSL_read(ssl, request + offset, toread);
        if (bytes <= 0) break;
        offset += bytes;
        toread -= bytes;
    }
    request[offset] = '\0';
    *outlen = offset;
    return request;
}



