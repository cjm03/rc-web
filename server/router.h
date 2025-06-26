#ifndef ROUTER_H
#define ROUTER_H

#include "parse.h"
#include "hashtable.h"

//=============================
// structs
//=============================

typedef struct {
    char* json;
    size_t offset;
} JsonBuffer;

//=============================
// functions
//=============================

/* appends clip information in json format to buffer */
void appendClipJson(Item* item, void* ctx);

/* creates the json file */
JsonBuffer bufJson(Table* t);

/* handles request from the URI passed into it. also takes in the table and client socket descriptor */
void handleRequest(Table* t, int client_fd, struct Request* req);

#endif // ROUTER_H
