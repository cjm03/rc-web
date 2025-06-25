#ifndef VIDEO_H
#define VIDEO_H

#include "hashtable.h"

#define BUFFER_SIZE 4096

/* serves a raw file at passed filepath to passed client socket descriptor */
void serveFile(int client_fd, const char* filepath);

/* serves a html page with the passed clip_id in mp4 format */
void serveClipPage(int client_fd, const char* clip_id);

/* serves a video file located at the clip_id's filepath to passed client socket descriptor */
/* also takes in the table and the Range header's range */
void serveVideo(Table* t, int client_fd, const char* clip_id, const char* range);

#endif // VIDEO_H
