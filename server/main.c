/*
 *  main.c
 *  
 *  Start the server, load metadata, and set up sockets
*/

#include <asm-generic/socket.h>
#include <openssl/bio.h>
#include <openssl/quic.h>
#include <openssl/tls1.h>
#include <stdint.h>
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
#include <pthread.h>

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

void* handleClient(void* arg)
{
    SSL_CTX* ctx = (SSL_CTX*)arg;
    BIO* clientbio = NULL;
    SSL* ssl = NULL;

    BIO* acceptor_bio = BIO_new_accept("8443");
    if (BIO_do_accept(acceptor_bio) <= 0) {
        SSL_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        pthread_exit(NULL);
    }
    clientbio = BIO_pop(acceptor_bio);

    if ((ssl = SSL_new(ctx)) == NULL) {
        ERR_print_errors_fp(stderr);
        warnx("error creating SSL handle for new connection");
        BIO_free(clientbio);
        BIO_free(acceptor_bio);
        pthread_exit(NULL);
    }
    SSL_set_bio(ssl, clientbio, clientbio);
    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        warnx("error performing SSL handshake");
        SSL_free(ssl);
        BIO_free(acceptor_bio);
        pthread_exit(NULL);
    }
    int reqlen = 0;
    char* buffer = readFullRequest(ssl, &reqlen);
    struct Request* req = parseRequest(buffer);
    if (!req) {
        fprintf(stderr, "parser fail\n");
        SSL_shutdown(ssl);
        SSL_free(ssl);
        BIO_free(acceptor_bio);
        free(buffer);
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&ut_mutex);
    int result = handleRequest(t, ut, ssl, req, "0.0.0.0", buffer);
    pthread_mutex_unlock(&ut_mutex);
    if (result != 0) {
        fprintf(stderr, "access [%s] denied\n", req->url);
    } else {
        printf("sent [%s]\n", req->url);
    }

    freeRequest(req);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    BIO_free(acceptor_bio);
    free(buffer);

    pthread_exit(NULL);
}

int main(void)
{
    int res = EXIT_FAILURE;
    BIO* acceptor_bio;

    /* Initialize and fill hash table */
    Table* t = createTable();
    loadClipsFromDir(t, "/data/mp4/rust");
    printf("Table: loaded\n");

    /* Initialize and load Users table */
    uTable* ut = createuTable();
    int tret = loadStore(ut, "store.txt");
    if (tret != 1) {
        fprintf(stderr, "error: loadStore\n");
        destroyTable(ut);
        exit(EXIT_FAILURE);
    }
    SSL_CTX* ctx = initSSLCTX();
    loadCerts(ctx, CERTFILE, KEYFILE);

    /* Initialize OpenSSL and SSL context */
    acceptor_bio = BIO_new_accept("8443");
    if (acceptor_bio == NULL) {
        SSL_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        errx(res, "failed to create acceptor bio");
    }

    BIO_set_bind_mode(acceptor_bio, BIO_BIND_REUSEADDR);
    if (BIO_do_accept(acceptor_bio) <= 0) {
        SSL_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        errx(res, "error: socket setup");
    }

    while (1) {
        BIO* clientbio;
        SSL* ssl;

        ERR_clear_error();

        if (BIO_do_accept(acceptor_bio) <= 0) continue;

        clientbio = BIO_pop(acceptor_bio);
        printf("Accepted new connection\n");

        if (fork() == 0) {

            if ((ssl = SSL_new(ctx)) == NULL) {
                ERR_print_errors_fp(stderr);
                warnx("error creating SSL handle for new connection");
                BIO_free(clientbio);
                continue;
            }
            SSL_set_bio(ssl, clientbio, clientbio);
            if (SSL_accept(ssl) <= 0) {
                ERR_print_errors_fp(stderr);
                warnx("error performing SSL handshake");
                SSL_free(ssl);
                continue;
            }
            int reqlen = 0;
            char* buffer = readFullRequest(ssl, &reqlen);
            struct Request* req = parseRequest(buffer);
            if (!req) {
                fprintf(stderr, "parser fail\n");
                SSL_shutdown(ssl);
                SSL_free(ssl);
                exit(EXIT_FAILURE);
            }
            if (handleRequest(t, ut, ssl, req, "0.0.0.0", buffer) != 0) {
                fprintf(stderr, "access [%s] denied\n", req->url);
                SSL_shutdown(ssl);
                SSL_free(ssl);
                exit(0);
            } else {
                printf("sent [%s]\n", req->url);
                freeRequest(req);
                SSL_shutdown(ssl);
                SSL_free(ssl);
                exit(0);
            }
        }
    }
    /* Clean */
    freeTable(t);
    destroyTable(ut);
    SSL_CTX_free(ctx);
    return 0;
}

