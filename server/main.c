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

/* Function to gather clips and place them in the hash table */
void loadClipsFromDir(const char* directory)
{
    DIR* dir = opendir(directory);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) {
            if (strstr(entry->d_name, ".mp4")) {
                char id[256] = {0};
                snprintf(id, sizeof(id), "%.*s",
                         (int)(strlen(entry->d_name) - 4), entry->d_name);

                char filepath[512];
                snprintf(filepath, sizeof(filepath), "%s/%s",
                         directory, entry->d_name);

                struct stat st;
                if (stat(filepath, &st) == 0) {
                    insertClip(id, filepath, st.st_size);
                    printf("Loaded clip: %s\n", id);
                }
            }
        }
    }
    closedir(dir);
}

int main(void)
{
    /* Initialize and fill hash table */
    printf("Initializing hashtable\n");
    initTable();
    printf("Loading videos\n");
    loadClipsFromDir("media");

    printf("\n\n\nStarting server...\n");

    /* Start server */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // assigns server_fd with a file descriptor referring to the endpoint created by socket()
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr); // converts 127.0.0.1 into binary network address structure then copies it into addr.sin_addr 
    addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) != 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    /* Server started successfully */
    printf("Server running on port %d\n", PORT);

    /* Deal with client connections to server */
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        char buffer[BUFFER_SIZE] = {0};

        read(client_fd, buffer, BUFFER_SIZE - 1);

        struct Request* req = parseRequest(buffer);
        if (req) printf("Testing Method: %d\n", req->method);
        handleRequest(client_fd, req);

        // printf("Request:\n%s\n", buffer);

        // handleRequest(client_fd, buffer);

        close(client_fd);
        freeRequest(req);
    }
    return 0;
}

