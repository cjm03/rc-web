#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "parse.h"

#define MAXLINELEN 256
#define MAXHDRLEN 256
#define PAIR 2
#define MAXHDRCOUNT 50
#define MAXBUF 4096


Request* parseRequest(const char* req_in)
{
    struct Request* req = NULL;
    req = malloc(sizeof(struct Request));
    if (!req) {
        return NULL;
    }
    memset(req, 0, sizeof(struct Request));

    // method
    size_t methodlen = strcspn(req_in, " ");
    if (memcmp(req_in, "GET", strlen("GET")) == 0) {
        req->method = GET;
    } else if (memcmp(req_in, "HEAD", strlen("HEAD")) == 0) {
        req->method = HEAD;
    } else if (memcmp(req_in, "POST", strlen("POST")) == 0) {
        req->method = POST;
    } else {
        req->method = UNSUPPORTED;
    }
    req_in += methodlen + 1;

    // uri
    size_t urllen = strcspn(req_in, " ");
    req->url = malloc(urllen + 1);
    if (!req->url) {
        freeRequest(req);
        return NULL;
    }
    memcpy(req->url, req_in, urllen);
    req->url[urllen] = '\0';
    req_in += urllen + 1;

    // http version
    size_t verlen = strcspn(req_in, "\r\n");
    req->version = malloc(verlen + 1);
    if (!req->version) {
        freeRequest(req);
        return NULL;
    }
    memcpy(req->version, req_in, verlen);
    req->version[verlen] = '\0';
    req_in += verlen + 2;

    struct Header* header = NULL, *last = NULL;
    while (req_in[0] != '\r' || req_in[1] != '\n') {
        last = header;
        header = malloc(sizeof(Header));
        if (!header) {
            freeRequest(req);
            return NULL;
        }

        // name
        size_t namelen = strcspn(req_in, ":");
        header->name = malloc(namelen + 1);
        if (!header->name) {
            freeRequest(req);
            return NULL;
        }
        memcpy(header->name, req_in, namelen);
        header->name[namelen] = '\0';
        req_in += namelen + 1;
        while (*req_in == ' ') {
            req_in++;
        }

        // value
        size_t valuelen = strcspn(req_in, "\r\n");
        header->value = malloc(valuelen + 1);
        if (!header->value) {
            freeRequest(req);
            return NULL;
        }
        memcpy(header->value, req_in, valuelen);
        header->value[valuelen] = '\0';
        req_in += valuelen + 2;

        // next
        header->next = last;
    }
    req->headers = header;
    req_in += 2;

    size_t bodylen = strlen(req_in);
    req->body = malloc(bodylen + 1);
    if (!req->body) {
        freeRequest(req);
        return NULL;
    }
    memcpy(req->body, req_in, bodylen);
    req->body[bodylen] = '\0';

    return req;
}

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
    free(req->url);
    free(req->version);
    freeHeader(req->headers);
    free(req->body);
    free(req);
}
















