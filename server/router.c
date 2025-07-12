/*
 *  router.c
 *  Route paths
*/

//==========================================================================================================
// include 
//==========================================================================================================

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include "router.h"
#include "video.h"
#include "hashtable.h"
#include "utils.h"
#include "parse.h"
#include "respheaders.h"
#include "alccalc.h"
#include "../libflate/flate.h"

static int dont_remake_json = 1;

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

JsonBuffer bufJson(Table* t)
{
    char json[16384] = "[";
    JsonBuffer buf = { .json = json, .offset = 1 };

    iterateClips(t, appendClipJson, &buf);

    json[buf.offset++] = ']';
    json[buf.offset] = '\0';

    return buf;
}

//==========================================================================================================
// Handler
//==========================================================================================================

void handleRequest(Table* t, SSL* ssl, struct Request* req)
{
    // Logic to obtain specific headers HERE!!!
    const char* range = getHeaderValue(req, "Range");

    //======================================================
    // Handle GET
    //======================================================
    if (req->method == GET) {

        const char* resource = req->url;

        if (strncmp(resource, "/clip?id=", 9) == 0) {

            /* Here, were stripping the first three characters (id=) from the URI because their filenames lack them */
            const char* idparam = strstr(resource, "id=");

            if (idparam) {

                char clip_id[256];
                char raw_id[256];

                sscanf(idparam + 3, "%255[^ \r\n]", raw_id);

                /* No longer necessary, logistically, since I remove whitespace from all the clip names */
                /* Good. Cleansing the data. Security. */
                urldecode(clip_id, raw_id);

                serveVideo(t, ssl, clip_id, range);

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
                serveClipPage(ssl, clipid);
                return;
            }

        } else if (strncmp(resource, "/", 1) == 0 && strlen(resource) == 1) {

            serveFile(ssl, "public/index.html");
            return;

        /* Used by the function in index.html. Generates .json file displaying all the useful
           and/or desirable information stored in the hash table */
        } else if (strncmp(resource, "/api/clips", 10) == 0 && strlen(resource) == 10) {

            if (dont_remake_json == 0) {
                printf("WARN: json is being regenerated...\n");

                char json[32768] = "[";
                JsonBuffer bufNew = { .json = json, .offset = 1 };

                iterateClips(t, appendClipJson, &bufNew);

                json[bufNew.offset++] = ']';
                json[bufNew.offset] = '\0';

                char header[256];
                snprintf(header, sizeof(header), APP_OK, bufNew.offset);

                SSL_write(ssl, header, strlen(header));
                SSL_write(ssl, json, bufNew.offset);

                int f = open("clipslocal.json", O_WRONLY, 0644);
                write(f, bufNew.json, bufNew.offset);
                close(f);

                dont_remake_json = 1;

            } else {

                serveFile(ssl, "clipslocal.json");

            }

            return;

        } else if (strncmp(resource, "/public/alccalc.html", 20) == 0) {

            serveFile(ssl, "public/alccalc.html");
            return;

        } else if (strncmp(resource, "/favicon.ico", 12) == 0) {

            printf("favicon\n");
            SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));

            return;

        } else {

            printf("%s %s\n", NOT_FOUND, resource);

            SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));

            return;
        }
    } else if (req->method == POST) {

        const char* postresource = req->url;

        if (strncmp(postresource, "/discount", 9) == 0) {

            char* posted = req->body;
            Discount* t = create();
            parseInput(t, posted);
            calcDisc(t);
            free(posted);

            Flate* f = NULL;
            flateSetFile(&f, "public/discounted.html");

            for (int k = 0; k < ITEMS; k++) {

                /* temps to convert float to char */
                char* temp1 = malloc(16);
                char* temp2 = malloc(16);
                char* temp3 = malloc(16);
                if (temp1 == NULL || temp2 == NULL || temp3 == NULL) {
                    perror("malloc\n");
                    exit(EXIT_FAILURE);
                }

                /* put rounded float value in temps */
                snprintf(temp1, 16, "%.2f", t->orig[k]);
                snprintf(temp2, 16, "%.2f", t->disc[k]);
                snprintf(temp3, 16, "%.2f", t->newp[k]);

                /* load temps into flate equivalent */
                flateSetVar(f, "original", temp1, NULL);
                flateSetVar(f, "discount", temp2, NULL);
                flateSetVar(f, "new", temp3, NULL);
                flateSetVar(f, "disc", "", NULL);

                /* dump the flate vars */
                flateDumpTableLine(f, "disc");

                /* free temps */
                free(temp1);
                free(temp2);
                free(temp3);
            }

            /* temp */
            char* temptotaldisc = malloc(16);
            char* temptotalcost = malloc(16);

            /* store */
            snprintf(temptotaldisc, 16, "%.2f", t->totaldisc);
            snprintf(temptotalcost, 16, "%.2f", t->totalcost);

            /* load */
            flateSetVar(f, "totaldisc", temptotaldisc, NULL);
            flateSetVar(f, "totalcost", temptotalcost, NULL);

            /* free */
            free(temptotaldisc);
            free(temptotalcost);

            /* buffer to store filled template */
            char* buf = flatePage(f);

            /* free flate and discount table */
            flateFreeMem(f);
            freeDisc(t);
            
            /* header shit */
            size_t len = strlen(buf);
            char header[512];
            snprintf(header, sizeof(header),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n\r\n",
                    len);

            /* write */
            SSL_write(ssl, header, strlen(header));
            SSL_write(ssl, buf, len);

            free(buf);

            return;
        }
    } else {
        printf("%s NOT FOUND\n", req->url);
        SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));
        return;
    }
}
