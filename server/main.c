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

#include "router.h"
#include "hashtable.h"
#include "parse.h"
#include "utils.h"

#define PORT 8090
#define BUFFER_SIZE 4096

int main(void)
{
    /* Initialize and fill hash table */
    printf("Initializing hashtable\n");
    Table* t = createTable();
    printf("Initialized\n");
    printf("Loading videos\n");
    loadClipsFromDir(t, "/data/mp4/rust");
    printf("Loaded\n");

    /* Start server */
    printf("\nStarting server...\n");
    struct sockaddr_in addr, cliaddr;
    const int enable = 1;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) perror("setsockopt REUSEADDR");
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) perror("setsockopt REUSEPORT");

    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) != 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server started successfully\n");
    printf("\tRunning on port %d\n", PORT);

    socklen_t clilen = sizeof(cliaddr);

    /* Deal with client connections to server */
    while (1) {

        /* accept a connection */
        int client_fd = accept(server_fd, (struct sockaddr*)&cliaddr, &clilen);
        if (client_fd < 0) perror("accept");

        /* gather client IP */
        char clientip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cliaddr.sin_addr, clientip, INET_ADDRSTRLEN);
        logIP("Connection from %s:%d\n", clientip, ntohs(cliaddr.sin_port));
        // printf("New connection from %s:%d\n", clientip, ntohs(cliaddr.sin_port));

        if (fork() == 0) {

            close(server_fd);

            char buffer[BUFFER_SIZE] = {0};

            /* Read the HTTP request into buffer */
            read(client_fd, buffer, BUFFER_SIZE - 1);

            /* Parse the request and store in Request structure req */
            struct Request* req = parseRequest(buffer);

            /* Handle that thang */
            printf("\nHandling %s\n", req->url);
            handleRequest(t, client_fd, req);
            printf("Handled\t");

            /* Say bye */
            close(client_fd);
            printf("Closed\t");

            /* Release */
            freeRequest(req);
            printf("Freed\n");

            exit(0);

        } else {

            close(client_fd);

        }
    }
    freeTable(t);
    return 0;
}

