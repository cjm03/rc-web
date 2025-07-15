#ifndef VIDEO_H
#define VIDEO_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include "hashtable.h"

#define BUFFER_SIZE 4096

/* serves a raw file at passed filepath to passed client socket descriptor */
void serveFile(SSL* ssl, const char* filepath);

/* serves a html page with the passed clip_id in mp4 format */
void serveClipPage(SSL* ssl, const char* clip_id);

/* serves a favicon */
void serveFavicon(SSL* ssl, const char* imagepath);

/* serves a video file located at the clip_id's filepath to passed client socket descriptor */
/* also takes in the table and the Range header's range */
void serveVideo(Table* t, SSL* ssl, const char* clip_id, const char* range);

#endif // VIDEO_H
