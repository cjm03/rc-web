/*
 *  router.c
 *  Route paths
*/

//==========================================================================================================
// include 
//==========================================================================================================

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "router.h"
#include "video.h"
#include "hashtable.h"
#include "utils.h"
#include "parse.h"
#include "respheaders.h"

//==========================================================================================================
// Structs
//==========================================================================================================

typedef struct {
    char* json;
    size_t offset;
} JsonBuffer;

//==========================================================================================================
// Functions
//==========================================================================================================

void appendClipJson(Item* item, void* ctx)
{
    JsonBuffer* buf = ctx;
    if (buf->offset > 1) {
        buf->json[buf->offset++] = ',';
        buf->json[buf->offset++] = '\n';
    }
    int written = snprintf(buf->json + buf->offset, 16384 - buf->offset,
                           "{\"id\":\"%s\",\"filename\":\"%s\",\"size\":%zu}",
                           item->id, item->path, item->size);
    buf->offset += written;
}

//==========================================================================================================
// Handler
//==========================================================================================================

void handleRequest(Table* t, int client_fd, struct Request* req)
{
    //======================================================
    // Logic to obtain the request's range header
    //======================================================
    const char* range = getHeaderValue(req, "Range");
    //======================================================
    // Handle GET
    //======================================================
    if (req->method == GET) {

        const char* resource = req->url;

        /* Specific clip */
        if (strncmp(resource, "/clip?id=", 9) == 0) {

            const char* idparam = strstr(resource, "id=");
            /* Here, were stripping the first three characters (id=) from the URI because their filenames lack them */
            if (idparam) {

                char clip_id[256];
                char raw_id[256];

                sscanf(idparam + 3, "%255[^ \r\n]", raw_id);

                /* No longer necessary, logistically, since I remove whitespace from all the clip names */
                /* Good. Cleansing the data. Security. */
                urldecode(clip_id, raw_id);

                printf("Serving video: %s\n", clip_id);

                serveVideo(t, client_fd, clip_id, range);

                return;
            }
        /* Bad code, too hacky. Need to find a way to render both the html and mp4 in one route. Dont want to though */
        } else if (strncmp(resource, "/media?id=", 10) == 0 && strlen(resource) > 10) {

            const char* idparam = strstr(resource, "id=");
            /* you should make this wizardry into a function */
            if (idparam) {
                char clipid[256];
                char rawid[256];
                sscanf(idparam + 3, "%255[^ \r\n]", rawid);
                urldecode(clipid, rawid);
                serveClipPage(client_fd, clipid);
                return;
            }
        /* Not useful. Already store it in index.html, the only file using it. May need later */
        } else if (strncmp(resource, "/public/style.css", 17) == 0) {

            serveFile(client_fd, "public/style.css");
            return;

        } else if (strncmp(resource, "/", 1) == 0 && strlen(resource) == 1) {

            serveFile(client_fd, "public/index.html");
            return;

        /* Used by the function in index.html. Generates .json file displaying all the useful
           and/or desirable information stored in the hash table */
        } else if (strncmp(resource, "/api/clips", 10) == 0 && strlen(resource) == 10) {

            char json[16384] = "[";
            JsonBuffer buf = { .json = json, .offset = 1 };

            iterateClips(t, appendClipJson, &buf);

            json[buf.offset++] = ']';
            json[buf.offset] = '\0';

            char header[256];
            snprintf(header, sizeof(header), APP_OK, buf.offset);

            write(client_fd, header, strlen(header));
            write(client_fd, json, buf.offset);

            close(client_fd);

            // int f = open("clipslocal.json", O_WRONLY, 0644);
            // write(f, json, buf.offset);
            // close(f);

            return;

        } else {

            printf("%s %s\n", NOT_FOUND, resource);

            write(client_fd, NOT_FOUND, strlen(NOT_FOUND));

            close(client_fd);

            return;
        }
    }
    write(client_fd, NOT_FOUND, strlen(NOT_FOUND));
    return;
}
