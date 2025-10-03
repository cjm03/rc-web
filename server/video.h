#ifndef VIDEO_H
#define VIDEO_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include "hashtable.h"

// #define BUFFER_SIZE 8192
#define BUFFER_SIZE 8192
#define CHUNK_SIZE 8192

typedef struct {
    long start;
    long end;
    long totalsize;
    int israngereq;
} range_info_t;

/* */
void serveAnyFile(SSL* ssl, const char* filepath, const char* hdr);

/* serves a raw file at passed filepath to passed client socket descriptor */
void serveFile(SSL* ssl, const char* filepath);

/* serves a html page with the passed clip_id in mp4 format */
void serveClipPage(SSL* ssl, const char* clip_id);

/*  */
void serveHome(SSL* ssl, char* filepath, char* ip);

/* serves a favicon */
void serveFavicon(SSL* ssl, const char* imagepath);
void serveImage(SSL* ssl, const char* filepath);

int parseRangeHeader(const char* range_header, range_info_t* range, long filesize);
int sendResponseHeaders(SSL* ssl, range_info_t* range, const char* filepath);
int serve_mp4_file_ssl(SSL* ssl, const char* filepath, const char* range_header);
/* serves a video file located at the clip_id's filepath to passed client socket descriptor */
/* also takes in the table and the Range header's range */
void serveVideo(Table* t, SSL* ssl, const char* clip_id, char* range);

#endif // VIDEO_H
