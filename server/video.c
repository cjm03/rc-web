/*
 *  video.c
 *  Load video metadata and serve video files
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>

#include "video.h"
#include "hashtable.h"
#include "respheaders.h"

//==========================================================================================================
// define
//==========================================================================================================

#define BUFFER_SIZE 4096

//==========================================================================================================
// functions
//==========================================================================================================

void serveFile(int client_fd, const char* filepath)
{
    int file_fd = open(filepath, O_RDONLY);
    if (file_fd < 0) {
        write(client_fd, NOT_FOUND, strlen(NOT_FOUND));
        return;
    }
    //======================================================================================================
    // Perform shakedown on the file. 404 if DNE. Get trustworthy info from the system like filesize.
    // Now, content length might be accurate. sendfile() is still retarded
    //======================================================================================================
    struct stat st;
    fstat(file_fd, &st);

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), TXT_OK, (long long)st.st_size);

    write(client_fd, header, strlen(header));

    off_t offset = 0;
    off_t len = st.st_size;
    if (sendfile(client_fd, file_fd, &offset, len) == -1) {
        if (errno == EPIPE || errno == ECONNRESET) {
            printf("sendfile ok");
        } else {
            printf("SFErrno %d :: %s :: filepath: %s\n", errno, strerror(errno), filepath);
        }
    }
    close(file_fd);
    close(client_fd);
    return;
}

void serveClipPage(int client_fd, const char* clip_id)
{
    char vidurl[256];
    snprintf(vidurl, sizeof(vidurl), "/clip?id=%s", clip_id);

    char html[4096];
    snprintf(html, sizeof(html),
             "<!DOCTYPE html>"
             "<html lang='en'>"
             "<head>"
             "<meta charset='UTF-8'>"
             "<title>Clip Viewer</title>"
             "<style>"
             "body { font-family: sans-serif; background: #121212; color: #eee; padding: 1em; }"
             "nav { background-color: #333; color: white; }"
             "nav a { color: white; text-decoration: none; }"
             ".container { display: flex; justify-content: center; align-items: center; padding: 2rem; }"
             "video { width: 100%%; height: auto; border: 2px solid #ccc; border-radius: 8px; }"
             "</style>"
             "</head>"
             "<body>"
             "<nav><a href='/'>Home</a></nav>"
             "<h1>Now Playing</h1>"
             "<div class='container'>"
             "<video controls autoplay>"
             "<source src='%s' type='video/mp4'>"
             "Your browser does not support the video tag."
             "</video>"
             "</div>"
             "</body></html>", vidurl);

    size_t bodylen = strlen(html);

    //======================================================================================================
    // Bad code actually horrendous
    // should fix...
    //======================================================================================================

    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "Connection: keep-alive\r\n\r\n", bodylen);

    write(client_fd, header, strlen(header));
    write(client_fd, html, bodylen);
    close(client_fd);
}

void serveVideo(Table* t, int client_fd, const char* clip_id, const char* range)
{ // Devil function. Biggest hassle.
    /* Consult hash table for verification */
    Item* clip = getItem(t, clip_id);
    if (!clip) {
        write(client_fd, NOT_FOUND, strlen(NOT_FOUND));
        return;
    }

    /* Consult disk for second verification */
    int file_fd = open(clip->path, O_RDONLY);
    if (file_fd < 0) {
        write(client_fd, INTERNAL_ERROR, strlen(INTERNAL_ERROR));
        return;
    }

    /* Consult kernel for third verification */
    struct stat st;
    if (fstat(file_fd, &st) < 0) {
        perror("fstat");
        close(file_fd);
        return;
    }
    off_t filesize = st.st_size;
    off_t start = 0;            /* range min */
    off_t end = filesize - 1;   /* range max */

    if (range && strncmp(range, "bytes=", 6) == 0) {
        sscanf(range + 6, "%ld-%ld", &start, &end);
        if (end == 0 || end >= filesize) end = filesize - 1;
    }
    off_t contentlength = end - start + 1;
    lseek(file_fd, start, SEEK_SET);

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 206 Partial Content\r\n"
             "Content-Type: video/mp4\r\n"
             "Content-Length: %ld\r\n"
             "Content-Range: bytes %ld-%ld/%ld\r\n"
             "Accept-Ranges: bytes\r\n"
             "Connection: close\r\n\r\n",
             contentlength, start, end, filesize);
    // snprintf(header, sizeof(header), VID_OK, clip->size);
    // printf("Header for clip: %s\n%s\n", clip->id, header);
    // printf("clip->size: %zu | st.st_size: %ld\n", clip->size, st.st_size);
    write(client_fd, header, strlen(header));

    signal(SIGPIPE, SIG_IGN);

    char buffer[8192];
    off_t remaining = contentlength;
    ssize_t n;
    while (remaining > 0 && (n = read(file_fd, buffer, sizeof(buffer))) > 0) {
        if (n > remaining) n = remaining;
        if (write(client_fd, buffer, n) != n) break;
        remaining -= n;
    }

    close(file_fd);
    fprintf(stderr, "Handled %s [%ld-%ld]\n", clip_id, start, end);
    close(client_fd);
    return;
}
