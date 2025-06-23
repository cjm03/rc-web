#ifndef VIDEO_H
#define VIDEO_H

#include "hashtable.h"

#define BUFFER_SIZE 4096

void serveFile(int client_fd, const char* filepath);
void serveClipPage(int client_fd, const char* clip_id);
void serveVideo(Table* t, int client_fd, const char* clip_id, const char* range);

#endif
