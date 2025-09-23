/*
 *  main.c
 *  
 *  Start the server, load metadata, and set up sockets
*/

#include <asm-generic/socket.h>
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

#define PORT 443
#define BUFFER_SIZE 8192
#define CERTFILE "ssl/server.crt"
#define KEYFILE "ssl/server.key"

int main(void)
{

    /* Initialize and fill hash table */
    Table* t = createTable();
    loadClipsFromDir(t, "/data/mp4/rust");
    printf("Table: loaded\n");

    /* Initialize and load Users table */
    // UsersTable* ut = createNewUsersTable();
    // insertUser(ut, "crabby", "B0Jangle$", "cjmoye@iu.edu");

    /* Initialize OpenSSL and SSL context */
    SSL_CTX* ctx = initSSLCTX();
    loadCerts(ctx, CERTFILE, KEYFILE);

    /* Server essentials */
    struct sockaddr_in addr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    const int enable = 1;

    /* Create socket */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Socket options */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) perror("setsockopt REUSEADDR");
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) perror("setsockopt REUSEPORT");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) != 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server: success [%d]\n", PORT);


    /* Deal with client connections to server */
    while (1) {

        /* Accept */
        int client_fd = accept(server_fd, (struct sockaddr*)&cliaddr, &clilen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        /* LOG */
        char clientip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cliaddr.sin_addr, clientip, INET_ADDRSTRLEN);
        logIP("Connection from %s:%d\n", clientip, ntohs(cliaddr.sin_port));

        /* Concurrency */
        if (fork() == 0) {

            close(server_fd);

            /* Gen cert */
            SSL* ssl = SSL_new(ctx);
            if (!ssl) {
                fprintf(stderr, "SSL_new: failure\n");
                close(client_fd);
                exit(EXIT_FAILURE);
            }

            SSL_set_fd(ssl, client_fd);

            /* Attempt cert accept */
            if (SSL_accept(ssl) <= 0) {
                // ERR_print_errors_fp(stderr);
                SSL_free(ssl);
                close(client_fd);
                exit(1);
            }

            // char buffer[BUFFER_SIZE] = {0};
            // int bRead = SSL_read(ssl, buffer, BUFFER_SIZE - 1);
            // if (bRead <= 0) {
            //     ERR_print_errors_fp(stderr);
            //     SSL_shutdown(ssl);
            //     SSL_free(ssl);
            //     close(client_fd);
            //     exit(EXIT_FAILURE);
            // }
            // buffer[bRead] = '\0';

            printf("\nbegin: read -> %s\n", clientip);
            int reqlen = 0;
            char* buffer = readFullRequest(ssl, &reqlen);

            /* Parse the request and store in Request structure req */
            // printf("%s\n", buffer);
            struct Request* req = parseRequest(buffer);
            if (!req) {
                fprintf(stderr, "parser: fail\n");
                SSL_shutdown(ssl);
                SSL_free(ssl);
                close(client_fd);
                exit(EXIT_FAILURE);
            }
            printf("parsed\n");

            /* Handle that thang */
            handleRequest(t, ssl, req, clientip);
            printf("[%s]: handled\t", req->url);

            /* Free the request */
            freeRequest(req);
            printf("req: freed\t");

            /* Shutdown SSL, free SSL, close clientfd */
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_fd);
            printf("client: closed\n");
            printf("end\n\n");

            exit(0);

        } else {

            close(client_fd);

        }
    }

    /* Clean */
    freeTable(t);
    SSL_CTX_free(ctx);

    return 0;
}

