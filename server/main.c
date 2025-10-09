/*
 *  main.c
 *  
 *  Start the server, load metadata, and set up sockets
*/

#include <asm-generic/socket.h>
#include <openssl/bio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdarg.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include "router.h"
#include "hashtable.h"
#include "parse.h"
#include "utils.h"
#include "users.h"

#define PORT 8443
#define BUFFER_SIZE 8192
#define CERTFILE "ssl/server.crt"
#define KEYFILE "ssl/server.key"

static void vwarnx(const char* fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
}
static void errx(int status, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vwarnx(fmt, ap);
    va_end(ap);
    exit(status);
}
static void warnx(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vwarnx(fmt, ap);
    va_end(ap);
}

int main(void)
{
    int res = EXIT_FAILURE;
    const char* hostport = "8443";

    /* Initialize and fill hash table */
    Table* t = createTable();
    loadClipsFromDir(t, "/data/mp4/rust");
    printf("Table: loaded\n");

    /* Initialize and load Users table */
    uTable* ut = createuTable();
    int ret = loadStore(ut, "store.txt");
    if (ret != 1) {
        fprintf(stderr, "error: loadStore\n");
        destroyTable(ut);
        exit(EXIT_FAILURE);
    }

    /* Initialize OpenSSL and SSL context */
    SSL_CTX* ctx = initSSLCTX();
    loadCerts(ctx, CERTFILE, KEYFILE);

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    // Create listener socket wrapped in a BIO
    BIO* acceptor_bio = BIO_new_accept(hostport);
    if (acceptor_bio == NULL) {
        SSL_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        errx(res, "error: acceptor bio creation");
    }
    BIO_set_bind_mode(acceptor_bio, BIO_BIND_REUSEADDR);
    if (BIO_do_accept(acceptor_bio) <= 0) {
        SSL_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        errx(res, "error: acceptor socket setup");
    }

    while(1) {

        BIO* clientbio;
        SSL* ssl;

        ERR_clear_error();

        if (BIO_do_accept(acceptor_bio) <= 0) {
            continue;
        }

        clientbio = BIO_pop(acceptor_bio);
        fprintf(stderr, "New connection accepted\n");

        if ((ssl = SSL_new(ctx)) == NULL) {
            ERR_print_errors_fp(stderr);
            warnx("error: ssl handle creation for new connection");
            BIO_free(clientbio);
            continue;
        }
        SSL_set_bio(ssl, clientbio, clientbio);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            warnx("error: ssl handshake");
            SSL_free(ssl);
            continue;
        }

        int reqlen = 0;
        char* buf = readFullRequest(ssl, &reqlen);
        struct Request* req = parseRequest(buf);
        if (!req) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            errx(res, "error: parser failure");
        }
        if (handleRequest(t, ut, ssl, req, "0.0.0.0", buf) != 0) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            errx(res, "error: access [%s] denied", req->url);
        } else {
            fprintf(stderr, "sent [%s]\n", req->url);
            freeRequest(req);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            // exit(0);
        }

    }


    /* Deal with client connections to server */

    /* Clean */
    freeTable(t);
    destroyTable(ut);
    SSL_CTX_free(ctx);

    return 0;
}

