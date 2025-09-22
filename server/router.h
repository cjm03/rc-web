#ifndef ROUTER_H
#define ROUTER_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

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
void handleRequest(Table* t, SSL* ssl, struct Request* req, char* ip);
// void handleRequest(UsersTable* ut, Table* t, SSL* ssl, struct Request* req);

#endif // ROUTER_H
