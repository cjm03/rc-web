#ifndef ROUTER_H
#define ROUTER_H

#include "hashtable.h"

void handleRequest(int client_fd, const char* request);

#endif
