/*
 *  parse.c
 *  HTTP request parser
 *  Stores request data in a struct Request. 
 *  Request holds the method, the URI (url), the version, the body, and the headers:
 *      The headers are stored in a struct Header holding the header name, associated value, and a pointer to the next header.
*/
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "parse.h"
#include "utils.h"

#define BUFFER_SIZE 8096

//===============================================================================================================
// functions to get specific headers or their values.
//===============================================================================================================

Header* getHeaderItem(Request* req, const char* header)
{
    struct Header* cur = req->headers;
    while (cur != NULL) {
        if (strncmp(cur->name, header, strlen(header)) == 0) {
            return cur;
        } else {
            cur = cur->next;
        }
    }
    return NULL;
}

const char* getHeaderValue(Request* req, const char* header)
{
    char* value = NULL;
    struct Header* cur = req->headers;
    while (cur != NULL) {
        if (strncmp(cur->name, header, strlen(header)) == 0) {
            value = strdup(cur->value);
            return value;
        } else {
            cur = cur->next;
        }
    }
    return NULL;

}

//===========================================================================================================
// The Parser.
//===========================================================================================================

Request* parseRequest(const char* raw)
{
    struct Request* req = malloc(sizeof(struct Request));
    if (req == NULL) {
        fprintf(stderr, "req malloc failed");
        exit(EXIT_FAILURE);
    }
    memset(req, 0, sizeof(struct Request));

    const char* bodystart = strstr(raw, "\r\n\r\n");
    size_t seplen = 4;
    if (!bodystart) {
        bodystart = strstr(raw, "\n\n");
        seplen = 2;
    }
    if (!bodystart) {
        fprintf(stderr, "malformed request, bailing");
        freeRequest(req);
        return NULL;
        // exit(EXIT_FAILURE);
    }
    size_t headerlen = bodystart - raw;
    char* headersection = malloc(headerlen + 1);                        // MEMORY SWAPPED
    // char* headersection = malloc(sizeof(Header));                        // MEMORY SWAPPED
    if (!headersection) {
        fprintf(stderr, "headersection malloc failure\n");
        free(req);
        exit(EXIT_FAILURE);
    }
    memcpy(headersection, raw, headerlen);
    headersection[headerlen] = '\0';

    char* line = strtok(headersection, "\r\n");
    char method[16] = {0}, uri[256] = {0}, version[16] = {0};
    sscanf(line, "%15s %255s %15s", method, uri, version);

    if (strcmp(method, "GET") == 0) req->method = GET;
    else if (strcmp(method, "POST") == 0) req->method = POST;
    else if (strcmp(method, "HEAD") == 0) req->method = HEAD;
    else req->method = UNSUPPORTED;

    req->url = strdup(uri);
    req->version = strdup(version);

    Header* last = NULL;
    while ((line = strtok(NULL, "\r\n")) != NULL && strlen(line) > 0) {
        char* colon = strchr(line, ':');
        if (!colon) continue;
        size_t namelen = colon - line;
        char* name = malloc(namelen + 1);                               // MEMORY
        if (!name) {
            fprintf(stderr, "header name malloc failure\n");
            free(headersection);
            freeRequest(req);
            exit(EXIT_FAILURE);
        }
        strncpy(name, line, namelen);
        name[namelen] = '\0';

        char* value = colon + 1;
        while (*value == ' ') value++;

        Header* header = calloc(1, sizeof(Header));                     // MEMORY
        if (!header) {
            fprintf(stderr, "header malloc failure\n");
            free(name);
            free(headersection);
            freeRequest(req);
            exit(EXIT_FAILURE);
        }
        header->name = name;
        header->value = strdup(value);
        header->next = last;
        last = header;
    }
    req->headers = last;

    size_t contentlength = 0;
    Header* h = req->headers;
    while (h) {
        if (strcasecmp(h->name, "Content-Length") == 0) contentlength = atoi(h->value);
        h = h->next;
    }

    bodystart += seplen;
    size_t bodylen = strlen(bodystart);
    if (contentlength > 0 && bodylen > contentlength) bodylen = contentlength;
    char* body = calloc(bodylen + 1, 1);                          // MEMORY
    if (!body) {
        fprintf(stderr, "body calloc failure\n");
        free(headersection);
        freeRequest(req);
        exit(EXIT_FAILURE);
    }
    memcpy(body, bodystart, bodylen);
    char* decodedbody = malloc(bodylen * 3 + 1);                      // MEMORY
    if (!decodedbody) {
        fprintf(stderr, "decodedbody malloc failure\n");
        free(body);
        free(headersection);
        freeRequest(req);
        exit(EXIT_FAILURE);
    }
    urldecode(decodedbody, body);
    // printf("decoded: %s\n", decodedbody);
    free(body);

    req->body = decodedbody;
    free(headersection);
    return req;
}

//========================================================================================
// printer
//======================================================================================================

void printRequest(Request* req)
{
    struct Header* cur = req->headers;
    printf("Method: %u\n", req->method);
    printf("URI: %s\n", req->url);
    printf("Version: %s\n", req->version);
    while (cur != NULL) {
        printf("\t%s: %s\n", cur->name, cur->value);
        cur = cur->next;
    }
    printf("Body: %s\n", req->body);
}

//===================================================================================================================
// free
//===================================================================================================================

void freeHeader(struct Header* h)
{
    if (h) {
        free(h->name);
        free(h->value);
        freeHeader(h->next);
        free(h);
    }
}

void freeRequest(struct Request* req)
{
    if (!req) return;
    free(req->url);
    free(req->version);
    freeHeader(req->headers);
    free(req->body);
    free(req);
}
















