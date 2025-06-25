#ifndef PARSE_H
#define PARSE_H

#define MAXLINELEN 256
#define MAXHDRLEN 256
#define PAIR 2
#define MAXHDRCOUNT 50
#define MAXBUF 4096

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

//===========================================
// get functions
//===========================================

/* takes in the request to search and the header to search for */
/* Return: the header struct with the matching header */
Header* getHeaderItem(Request* req, const char* header);

/* same as above but returns just the header value */
const char* getHeaderValue(Request* req, const char* header);

//===========================================
// The parser
//===========================================

/* takes in the buffer containing the request to parse */
/* first obtains the method, then the URI, then the version, then parses all the headers */
Request* parseRequest(const char* req_in);

//===========================================
// free
//===========================================

/* takes in the header struct to free */
void freeHeader(struct Header *h);

/* takes in the request struct to free */
/* first frees all headers, then frees the request struct */
void freeRequest(struct Request* req);

#endif
