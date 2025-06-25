/*
 *  main.c
 *  
 *  Start the server, load metadata, and set up sockets
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <stdbool.h>
#include <dirent.h>

#include "router.h"
#include "hashtable.h"
#include "parse.h"

#define PORT 8080
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
    printf("\n\n\nStarting server...\n");
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // assigns server_fd with a file descriptor referring to the endpoint created by socket()
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr); // converts 127.0.0.1 into binary network address structure then copies it into addr.sin_addr 
    addr.sin_port = htons(PORT);

    /* Bind */
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    /* Listen */
    if (listen(server_fd, 10) != 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    /* Server started successfully */
    printf("Server running on port %d\n", PORT);

    /* Deal with client connections to server */
    while (1) {

        /* accept a connection */
        int client_fd = accept(server_fd, NULL, NULL);

        /* if accepted: */
        if (client_fd != -1) {

            char buffer[BUFFER_SIZE] = {0};

            /* Read the HTTP request into buffer */
            read(client_fd, buffer, BUFFER_SIZE - 1);

            /* Parse the request and store in Request structure req */
            struct Request* req = parseRequest(buffer);

            /* Handle that thang */
            printf("Handling\n");
            handleRequest(t, client_fd, req);
            printf("Handled\n");

            /* Say bye */
            close(client_fd);
            printf("Closed client_fd\n");

            /* Release */
            freeRequest(req);
            printf("Freed req\n");
        }
    }
    freeTable(t);
    return 0;
}

