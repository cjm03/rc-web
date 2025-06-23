#ifndef ROUTER_H
#define ROUTER_H

#include "parse.h"
#include "hashtable.h"

void handleRequest(Table* t, int client_fd, struct Request* req);
void appendClipJson(Item* item, void* ctx);

#endif
