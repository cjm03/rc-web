/*
 *  router.c
 *  
 *  Route paths
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "router.h"
#include "video.h"
#include "hashtable.h"


void handleRequest(int client_fd, const char* request, htClip* ht)
{
    if (strncmp(request, "GET /clip?", 10) == 0) {
        const char* id_param = strstr(request, "id=");
        printf("\nSTRSTR: %s\n", id_param);
        if (id_param) {
            int start = 3;
            int end = 11;
            int length = end - start;
            char* clip_id = malloc(length + 1);
            strncpy(clip_id, id_param + start, length);
            clip_id[length] = '\0';
            printf("\nSTRNCPY: '%s'\n", clip_id);
            // sscanf(id_param + 3, "%255[^ \r\n]", clip_id);
            serveVideo(client_fd, clip_id);
            free(clip_id);
            return;
        }
    }

    if (strncmp(request, "GET / ", 6) == 0 || strncmp(request, "GET /HTTP", 9) == 0) {
        serveFile(client_fd, "public/index.html");
        return;
    }

    if (strncmp(request, "GET /api/clips", 14) == 0) {
        char json[8192] = "[";
        int first = 1;
        for (int i = 0; i < TABLE_SIZE; i++) {
            htClip* cur = &ht[i];
            while (cur) {
                if (!first)
                    strcat(json, ",");
                char entry[512];
                snprintf(entry, sizeof(entry),
                         "{\"id\":\"%s\",\"filename\":\"%s\",\"size\":\"TODOOOOO\"}",
                         cur->key, cur->value);
                strcat(json, entry);
                first = 0;
            }
        }
        strcat(json, "]");

        char header[256];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %zu\r\n"
                 "Connection: close\r\n\r\n",
                 strlen(json));

        write(client_fd, header, strlen(header));
        write(client_fd, json, strlen(json));
        return;
    }

    const char* not_found = "HTTP/1.1 404 Not Found\r\n\r\nNot Found";
    write(client_fd, not_found, strlen(not_found));
}
