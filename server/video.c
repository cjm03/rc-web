/*
 *  video.c
 *  Load video metadata and serve video files
*/

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <signal.h>

#include "video.h"
#include "hashtable.h"
#include "respheaders.h"
#include "libflate/flate.h"

//==========================================================================================================
// define
//==========================================================================================================

#define CHUNK_SIZE 8192

//==========================================================================================================
// functions
//==========================================================================================================

void serveAnyFile(SSL* ssl, const char* filepath, const char* hdr)
{
    int file_fd = open(filepath, O_RDONLY);
    if (file_fd < 0) {
        SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));
        return;
    }
    
    struct stat st;
    fstat(file_fd, &st);

    char header[BUFFER_SIZE];

    snprintf(header, sizeof(header), hdr, (long long)st.st_size);

    SSL_write(ssl, header, strlen(header));

    char buffer[CHUNK_SIZE];
    ssize_t bRead, bWrite;
    off_t totalSent = 0;

    while ((bRead = read(file_fd, buffer, CHUNK_SIZE)) > 0) {
        ssize_t sent = 0;
        while (sent < bRead) {
            bWrite = SSL_write(ssl, buffer + sent, bRead - sent);
            if (bWrite <= 0) {
                ERR_print_errors_fp(stderr);
                break;
            }
            sent += bWrite;
            totalSent += bWrite;
        }
    }

    close(file_fd);
    return;
}


void serveFile(SSL* ssl, const char* filepath)
{
    int file_fd = open(filepath, O_RDONLY);
    if (file_fd < 0) {
        SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));
        return;
    }
    //======================================================================================================
    // Perform shakedown on the file. 404 if DNE. 
    // Get trustworthy info from the system like filesize.
    // Now, content length might be accurate. sendfile() is still retarded
    //======================================================================================================
    struct stat st;
    fstat(file_fd, &st);

    char header[BUFFER_SIZE];

    int fplen = strlen(filepath);
    char* look = strstr(filepath, ".css");
    if (look && filepath[fplen - 4] == '.') {

        snprintf(header, sizeof(header), CSS_OK, (long long)st.st_size);

    } else {
        snprintf(header, sizeof(header), TXT_OK, (long long)st.st_size);
    }

    SSL_write(ssl, header, strlen(header));

    char buffer[CHUNK_SIZE];
    ssize_t bRead, bWrite;
    off_t totalSent = 0;

    while ((bRead = read(file_fd, buffer, CHUNK_SIZE)) > 0) {
        ssize_t sent = 0;
        while (sent < bRead) {
            bWrite = SSL_write(ssl, buffer + sent, bRead - sent);
            if (bWrite <= 0) {
                ERR_print_errors_fp(stderr);
                break;
            }
            sent += bWrite;
            totalSent += bWrite;
        }
    }

    close(file_fd);
    return;
}

void serveHome(SSL* ssl, char* filepath, char* ip)
{
    Flate* f = NULL;
    // flateSetFile(&f, "public/home.html");
    flateSetFile(&f, filepath);
    flateSetVar(f, "yoip", ip, NULL);
    char* buffa = flatePage(f);
    flateFreeMem(f);

    size_t len = strlen(buffa);
    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %ld\r\n"
             "Connection: keep-alive\r\n\r\n", len);
    SSL_write(ssl, header, strlen(header));
    SSL_write(ssl, buffa, len);
    free(buffa);
    return;

}

void serveClipPage(SSL* ssl, const char* clip_id, size_t size)
{
    char vidurl[256];
    snprintf(vidurl, sizeof(vidurl), "/clip?id=%s", clip_id);

    Flate* f = NULL;
    flateSetFile(&f, "public/clip.html");
    flateSetVar(f, "clipname", clip_id, NULL);
    flateSetVar(f, "clip", vidurl, NULL);

    char* buf = flatePage(f);
    flateFreeMem(f);

    size_t len = strlen(buf);
    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %ld\r\n"
             "Accept-Ranges: bytes\r\n"
             "Connection: keep-alive\r\n\r\n", len);

    SSL_write(ssl, header, strlen(header));
    SSL_write(ssl, buf, len);
    free(buf);
    return;
}

void serveFavicon(SSL* ssl, const char* imagepath)
{
    int ico = open(imagepath, O_RDONLY);
    if (ico < 0) {
        SSL_write(ssl, INTERNAL_ERROR, strlen(INTERNAL_ERROR));
        return;
    }
    struct stat st;
    if (fstat(ico, &st) < 0) {
        perror("fstat (serveFavicon)");
        close(ico);
        return;
    }
    off_t filesize = st.st_size;
    char header[512];
    snprintf(header, sizeof(header), ICO_OK, filesize);
    SSL_write(ssl, header, strlen(header));
    char wombat[16384];
    read(ico, wombat, sizeof(wombat));
    SSL_write(ssl, wombat, filesize);
    close(ico);
    return;
}

void serveImage(SSL* ssl, const char* filepath)
{
    int img = open(filepath, O_RDONLY);
    if (img < 0) {
        SSL_write(ssl, INTERNAL_ERROR, strlen(INTERNAL_ERROR));
        return;
    }
    struct stat st;
    if (fstat(img, &st) < 0) {
        perror("fstat (serveImage)");
        close(img);
        return;
    }
    off_t filesize = st.st_size;
    char header[512];
    snprintf(header, sizeof(header), ICO_OK, filesize);
    SSL_write(ssl, header, strlen(header));
    char storethathoe[filesize + 1];
    read(img, storethathoe, sizeof(storethathoe));
    SSL_write(ssl, storethathoe, filesize);
    close(img);
    return;
}


void serveVideo(Table* t, SSL* ssl, const char* clip_id, const char* range)
{ // Devil function. Biggest hassle.
    /* Consult hash table for verification */
    Item* clip = getItem(t, clip_id);
    printf("%s\n", clip->path);
    if (!clip) {
        SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));
        return;
    }

    /* Consult disk for second verification */
    int file_fd = open(clip->path, O_RDONLY);
    if (file_fd < 0) {
        SSL_write(ssl, INTERNAL_ERROR, strlen(INTERNAL_ERROR));
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
    off_t request_start = 0;        
    off_t request_end;          

    ssize_t bRead = 0;
    ssize_t bWrite = 0;

    printf("Range: %s\n", range);
    char* token = strchr(range, '-');
    *token = ' ';
    printf("Range: %s\n", range);
    int verify = sscanf(range, "%ld %ld", &request_start, &request_end);

    if (verify != 2) {
        request_end = 0;
    }

    ssize_t requestlength = request_end - request_start + 1;
    lseek(file_fd, request_start, SEEK_SET);                                

    char header[BUFFER_SIZE];                                               
    snprintf(header, sizeof(header), PARTIAL, requestlength, request_start, request_end, filesize);
    SSL_write(ssl, header, strlen(header));

    signal(SIGPIPE, SIG_IGN);

    char buffer[CHUNK_SIZE];
    memset(buffer, 0, CHUNK_SIZE);

    ssize_t rem = requestlength;
    while (rem > 0 && (bRead = read(file_fd, buffer, sizeof(buffer))) > 0) {
        ssize_t sent = 0;
        while (sent < bRead) {
            bWrite = SSL_write(ssl, buffer + sent, bRead - sent);
            if (bWrite <= 0) break;
            sent += bWrite;
        }
    }
    printf("Read: %lu\nWrite: %lu\n", bRead, bWrite);

    close(file_fd);
    return;
}
