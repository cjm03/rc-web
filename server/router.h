#ifndef ROUTER_H
#define ROUTER_H

#include "parse.h"
#include "hashtable.h"

// typedef struct {
//     char* json;
//     size_t offset;
// } JsonBuffer;

// also unsure of how this works exactly
void appendClipJson(Clip* clip, void* ctx);

/* Function to determine appropriate way to respond to client request 
 * Current Possibilities:
 *      GET /           
 *      GET /clip?      
 *      GET /api/clips  
*/

void handleRequest(int client_fd, struct Request* req);
// void handleRequest(int client_fd, const char* request);

#endif
