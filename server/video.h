#ifndef VIDEO_H
#define VIDEO_H

void serveFile(int client_fd, const char* filepath);
void serveVideo(int client_fd, const char* clip_id);

#endif
