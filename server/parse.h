#ifndef PARSE_H
#define PARSE_H

typedef enum Method {UNSUPPORTED, GET, HEAD, POST} Method;
typedef struct Header {
    char* name;
    char* value;
    struct Header* next;
} Header;
typedef struct Request {
    Method method;
    char* url;
    char* version;
    Header* headers;
    char* body;
} Request;


Request* parseRequest(const char* req_in);
void freeHeader(struct Header *h);
void freeRequest(struct Request* req);

#endif
