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
#include "libflate/flate.h"

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

void handleRequest(Table* t, SSL* ssl, struct Request* req, char* ip)
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


            if (idparam) {
                char clipid[256];
                char rawid[256];
                sscanf(idparam + 3, "%255[^ \r\n]", rawid);
                urldecode(clipid, rawid);
                Item* i = getItem(t, clipid);
                serveClipPage(ssl, clipid, i->size); // , t->items->);
                return;
            }

        } else if (strncmp(resource, "/", 1) == 0 && strlen(resource) == 1) {

            serveHome(ssl, "public/home.html", ip);
            // serveFile(ssl, "public/home.html");
            return;

        } else if (strncmp(resource, "/public/css/bootstrap.css", 25) == 0) {

            serveFile(ssl, "public/css/bootstrap.css");
            return;

        } else if (strncmp(resource, "/public/css/bootstrap.css.map", 29) == 0) {

            serveFile(ssl, "public/css/bootstrap.css.map");
            return; 

        } else if (strncmp(resource, "/clipindex.html", 15) == 0) {

            serveAnyFile(ssl, "public/clipindex.html", TXT_OK);
            // serveFile(ssl, "public/clipindex.html");
            return;

        } else if (strncmp(resource, "/api/clips", 10) == 0 && strlen(resource) == 10) {

            if (dont_remake_json == 0) {

                fprintf(stderr, "WARN: json is being regenerated...\n");
                char json[32768] = "[";
                JsonBuffer bufNew = { .json = json, .offset = 1 };
                iterateClips(t, appendClipJson, &bufNew);
                json[bufNew.offset++] = ']';
                json[bufNew.offset] = '\0';
                char header[256];
                snprintf(header, sizeof(header), APP_OK, bufNew.offset);
                SSL_write(ssl, header, strlen(header));
                SSL_write(ssl, json, bufNew.offset);
                int f = open("clips.json", O_WRONLY, 0644);
                write(f, bufNew.json, bufNew.offset);
                close(f);
                dont_remake_json = 1;

            } else {

                serveFile(ssl, "public/clips.json");

            }

            return;

        } else if (strncmp(resource, "/public/alccalc.html", 20) == 0) {

            serveFile(ssl, "public/alccalc.html");
            return;

        } else if (strncmp(resource, "/public/favicon.png", 19) == 0) {

            serveFavicon(ssl, "public/favicon.png");
            return;

        } else if (strncmp(resource, "/private/phishy.png", 19) == 0) {

            serveImage(ssl, "private/phishy.png");
            return;

        } else if (strncmp(resource, "/private/basket.png", 19) == 0) {

            serveImage(ssl, "private/basket.png");
            return;

        } else if (strncmp(resource, "/private/meitei.png", 19) == 0) {

            serveImage(ssl, "private/meitei.png");
            return;

        } else if (strncmp(resource, "/private/gnome.jpeg", 19) == 0) {

            serveImage(ssl, "private/gnome.jpeg");
            return;

        } else {

            printf("%s %s ", "error:", resource);
            SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));
            return;
        }
    } else if (req->method == POST) {

        const char* postresource = req->url;

        if (strncmp(postresource, "/discount", 9) == 0) {

            // printRequest(req);

            Discount* t = createDiscountTable();
            parseDiscountInput(t, req->body);
            calculateDiscount(t);

            Flate* f = NULL;
            flateSetFile(&f, "public/discounted.html");

            for (int k = 0; k < ITEMS; k++) {
                /* temps to convert float to char */
                char buffer[3];
                char* temp1 = malloc(16);
                char* temp2 = malloc(16);
                char* temp3 = malloc(16);
                if (temp1 == NULL || temp2 == NULL || temp3 == NULL) {
                    perror("malloc\n");
                    exit(EXIT_FAILURE);
                }

                /* put rounded float value in temps */
                sprintf(buffer, "%d", k + 1);
                snprintf(temp1, 16, "%.2f", t->orig[k]);
                snprintf(temp2, 16, "%.2f", t->newp[k]);
                snprintf(temp3, 16, "%.2f", t->disc[k]);

                /* load temps into flate equivalent */
                flateSetVar(f, "indexnum", buffer, NULL);
                flateSetVar(f, "original", temp1, NULL);
                flateSetVar(f, "new", temp2, NULL);
                flateSetVar(f, "discount", temp3, NULL);
                flateSetVar(f, "disc", "", NULL);

                /* dump the flate vars */
                flateDumpTableLine(f, "disc");

                /* free temps */
                free(temp1);
                free(temp2);
                free(temp3);
            }

            /* temp */
            char* temptotalorig = malloc(16);
            char* temptotaldisc = malloc(16);
            char* temptotalcost = malloc(16);
            if (temptotalorig == NULL || temptotaldisc == NULL || temptotalcost == NULL) {
                perror("malloc\n");
                exit(EXIT_FAILURE);
            }

            /* store */
            snprintf(temptotalorig, 16, "%.2f", t->totalorig);
            snprintf(temptotaldisc, 16, "%.2f", t->totaldisc);
            snprintf(temptotalcost, 16, "%.2f", t->totalcost);

            /* load */
            flateSetVar(f, "totalorig", temptotalorig, NULL);
            flateSetVar(f, "totaldisc", temptotaldisc, NULL);
            flateSetVar(f, "totalcost", temptotalcost, NULL);

            /* free */
            free(temptotalorig);
            free(temptotaldisc);
            free(temptotalcost);

            /* buffer to store filled template */
            char* buf = flatePage(f);

            /* free flate and discount table */
            flateFreeMem(f);
            freeDiscountTable(t);
            
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

        printf("%s %s ", "error:", req->url);
        SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));
        return;

    }
}
