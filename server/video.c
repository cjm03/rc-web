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
// #include <signal.h>

#include "video.h"
#include "hashtable.h"
#include "respheaders.h"
#include "libflate/flate.h"

//==========================================================================================================
// define
//==========================================================================================================

#define BIGBUFFER_SIZE 65536
#define MAXHEADERS 2048

//==========================================================================================================
// structures
//==========================================================================================================


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

    char buffer[BUFFER_SIZE];
    ssize_t bRead, bWrite;
    off_t totalSent = 0;

    while ((bRead = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
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

    char buffer[BUFFER_SIZE];
    ssize_t bRead, bWrite;
    off_t totalSent = 0;

    while ((bRead = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
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

void serveClipPage(SSL* ssl, const char* clip_id)
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

int parseRangeHeader(const char* range_header, range_info_t* range, long filesize)
{
    if (!range_header || strncmp(range_header, "bytes=", 6) != 0) {
        range->israngereq = 0;
        range->start = 0;
        range->end = filesize - 1;
        range->totalsize = filesize;
        return 0;
    }
    range->israngereq = 1;
    range->totalsize = filesize;

    const char* rangespec = range_header + 6;
    char* dash = strchr(rangespec, '-');
    if (!dash) return -1;

    if (dash == rangespec) {
        range->end = filesize - 1;
        range->start = filesize - atol(dash + 1);
        if (range->start < 0) range->start = 0;
    } else {
        range->start = atol(rangespec);
        if (*(dash + 1) == '\0') {
            range->end = filesize - 1;
        } else {
            range->end = atol(dash + 1);
        }
    }
    if (range->start >= filesize) return -1;
    if (range->end >= filesize) range->end = filesize - 1;
    if (range->start > range->end) return -1;
    return 0;
}

int sendResponseHeaders(SSL* ssl, range_info_t* range, const char* filepath)
{
    char headers[MAXHEADERS];
    int header_len;
    
    if (range->israngereq) {
        // 206 Partial Content response
        header_len = snprintf(headers, sizeof(headers),
            "HTTP/1.1 206 Partial Content\r\n"
            "Content-Type: video/mp4\r\n"
            "Accept-Ranges: bytes\r\n"
            "Content-Length: %ld\r\n"
            "Content-Range: bytes %ld-%ld/%ld\r\n"
            "Cache-Control: public, max-age=3600\r\n"
            "Connection: keep-alive\r\n"
            "\r\n",
            range->end - range->start + 1,
            range->start,
            range->end,
            range->totalsize
        );
    } else {
        // 200 OK response
        header_len = snprintf(headers, sizeof(headers),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: video/mp4\r\n"
            "Accept-Ranges: bytes\r\n"
            "Content-Length: %ld\r\n"
            "Cache-Control: public, max-age=3600\r\n"
            "Connection: keep-alive\r\n"
            "\r\n",
            range->totalsize
        );
    }
    
    int bytes_sent = 0;
    int total_sent = 0;
    
    while (total_sent < header_len) {
        bytes_sent = SSL_write(ssl, headers + total_sent, header_len - total_sent);
        if (bytes_sent <= 0) {
            int ssl_error = SSL_get_error(ssl, bytes_sent);
            fprintf(stderr, "SSL_write error in headers: %d\n", ssl_error);
            return -1;
        }
        total_sent += bytes_sent;
    }
    
    return 0;
}

int serve_mp4_file_ssl(SSL* ssl, const char* filepath, const char* range_header) {
    FILE* file = NULL;
    unsigned char* buffer = NULL;
    int result = -1;
    
    // Validate inputs
    if (!ssl || !filepath) {
        fprintf(stderr, "Invalid SSL connection or filepath\n");
        return -1;
    }
    
    // Open file
    file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s - %s\n", filepath, strerror(errno));
        
        // Send 404 Not Found
        const char* not_found = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Length: 13\r\n\r\n"
                               "File not found";
        SSL_write(ssl, not_found, strlen(not_found));
        return -1;
    }
    
    // Get file size
    struct stat file_stat;
    if (fstat(fileno(file), &file_stat) != 0) {
        fprintf(stderr, "Failed to get file stats: %s\n", strerror(errno));
        fclose(file);
        return -1;
    }
    
    long file_size = file_stat.st_size;
    printf("Serving MP4 file: %s (%.2f MB)\n", filepath, file_size / (1024.0 * 1024.0));
    
    // Parse range request
    range_info_t range;
    if (parseRangeHeader(range_header, &range, file_size) != 0) {
        fprintf(stderr, "Invalid range request\n");
        
        // Send 416 Range Not Satisfiable
        const char* range_error = "HTTP/1.1 416 Range Not Satisfiable\r\n"
                                 "Content-Range: bytes */";
        char range_response[256];
        snprintf(range_response, sizeof(range_response), 
                "%s%ld\r\n\r\n", range_error, file_size);
        SSL_write(ssl, range_response, strlen(range_response));
        fclose(file);
        return -1;
    }
    
    // Send response headers
    if (sendResponseHeaders(ssl, &range, filepath) != 0) {
        fprintf(stderr, "Failed to send headers\n");
        fclose(file);
        return -1;
    }
    
    // Allocate buffer
    buffer = malloc(BIGBUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer\n");
        fclose(file);
        return -1;
    }
    
    // Seek to start position
    if (fseek(file, range.start, SEEK_SET) != 0) {
        fprintf(stderr, "Failed to seek to position %ld\n", range.start);
        free(buffer);
        fclose(file);
        return -1;
    }
    
    // Send file data
    long bytes_to_send = range.end - range.start + 1;
    long total_sent = 0;
    
    printf("Sending bytes %ld-%ld (%ld bytes total)\n", 
           range.start, range.end, bytes_to_send);
    
    while (total_sent < bytes_to_send) {
        // Calculate how much to read this iteration
        size_t chunk_size = BIGBUFFER_SIZE;
        if (bytes_to_send - total_sent < chunk_size) {
            chunk_size = bytes_to_send - total_sent;
        }
        
        // Read from file
        size_t bytes_read = fread(buffer, 1, chunk_size, file);
        if (bytes_read == 0) {
            if (feof(file)) {
                printf("Reached end of file\n");
                break;
            } else {
                fprintf(stderr, "File read error: %s\n", strerror(errno));
                break;
            }
        }
        
        // Send via SSL
        size_t bytes_sent = 0;
        while (bytes_sent < bytes_read) {
            int ssl_result = SSL_write(ssl, buffer + bytes_sent, bytes_read - bytes_sent);
            if (ssl_result <= 0) {
                int ssl_error = SSL_get_error(ssl, ssl_result);
                if (ssl_error == SSL_ERROR_WANT_WRITE) {
                    // Non-blocking socket would block, try again
                    continue;
                } else {
                    fprintf(stderr, "SSL_write error: %d\n", ssl_error);
                    ERR_print_errors_fp(stderr);
                    goto cleanup;
                }
            }
            bytes_sent += ssl_result;
        }
        
        total_sent += bytes_sent;
        
        // Progress indicator for large files
        if (total_sent % (10 * 1024 * 1024) == 0) { // Every 10MB
            printf("Sent: %.2f MB / %.2f MB (%.1f%%)\n", 
                   total_sent / (1024.0 * 1024.0),
                   bytes_to_send / (1024.0 * 1024.0),
                   (total_sent * 100.0) / bytes_to_send);
        }
    }
    
    printf("Transfer complete. Sent %ld bytes\n", total_sent);
    result = 0; // Success
    
cleanup:
    if (buffer) free(buffer);
    if (file) fclose(file);
    return result;
}


void serveVideo(Table* t, SSL* ssl, const char* clip_id, char* range)
{
    char* rangeheader = NULL;
    char* rangeline = strstr(range, "Range: ");
    if (rangeline) {
        rangeline += 7;
        char* lineend = strstr(rangeline, "\r\n");
        if (lineend) {
            int rangelen = lineend - rangeline;
            rangeheader = malloc(rangelen + 1);
            strncpy(rangeheader, rangeline, rangelen);
            rangeheader[rangelen] = '\0';
        }
    }
    Item* clip = getItem(t, clip_id);
    if (!clip) {
        SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));
        if (rangeheader) {
            free(rangeheader);
        }
        return;
    }
    int result = serve_mp4_file_ssl(ssl, clip->path, rangeheader);
    if (result != 0) fprintf(stderr, "failure\n");
    if (rangeheader) free(rangeheader);
    return;
}



// { // Devil function. Biggest hassle.
//     /* Consult hash table for verification */
//     Item* clip = getItem(t, clip_id);
//     printf("%s\n", clip->path);
//     if (!clip) {
//         SSL_write(ssl, NOT_FOUND, strlen(NOT_FOUND));
//         return;
//     }
//
//     /* Consult disk for second verification */
//     // int file_fd = open(clip->path, O_RDONLY);
//     int file_fd = open(clip->path, O_RDONLY);
//     if (file_fd < 0) {
//         SSL_write(ssl, INTERNAL_ERROR, strlen(INTERNAL_ERROR));
//         return;
//     }
//
//     /* Consult kernel for third verification */
//     struct stat st;
//     if (fstat(file_fd, &st) < 0) {
//         perror("fstat");
//         close(file_fd);
//         return;
//     }
//
//     off_t filesize = st.st_size;
//     off_t request_start = 0;        
//     off_t request_end;          
//
//     ssize_t bRead = 0;
//     ssize_t bWrite = 0;
//
//     printf("Range: %s\n", range);
//     char* token = strchr(range, '-');
//     *token = ' ';
//     if (strlen(range) <= 2) {
//         sscanf(range, "%ld", &request_start);
//         request_end = filesize;
//         printf("%ld-%ld\n", request_start, request_end);
//     }
//     // printf("Range: %lu\n", strlen(range));
//     sscanf(range, "%ld %ld", &request_start, &request_end);
//
//     ssize_t requestlength = request_end - request_start + 1;
//     lseek(file_fd, request_start, SEEK_SET);                                
//
//     char header[BUFFER_SIZE];                                               
//     snprintf(header, sizeof(header), PARTIAL, requestlength, request_start, request_end, filesize);
//     SSL_write(ssl, header, strlen(header));
//
//     signal(SIGPIPE, SIG_IGN);
//
//     char buffer[CHUNK_SIZE];
//     memset(buffer, 0, CHUNK_SIZE);
//
//     ssize_t rem = requestlength;
//
//     while ((bRead = read(file_fd, buffer, sizeof(buffer))) > 0) {
//         if (SSL_write(ssl, buffer, bRead) < 0) {
//             fprintf(stderr, "write failed");
//             rem = rem - bWrite;
//             close(file_fd);
//             return;
//             // lseek(file_fd, bWrite, SEEK_SET);
//             // memset(buffer, 0, CHUNK_SIZE);
//         }
//     }
//
//     // while (rem > 0 && (bRead = read(file_fd, buffer, sizeof(buffer) - 1)) > 0) {
//     // // while (rem > 0 && (bRead = fread(buffer, sizeof(char), sizeof(buffer), file_fd)) > 0) {
//     //     ssize_t sent = 0;
//     //     while (sent < bRead) {
//     //         sent = SSL_write(ssl, buffer + sent, bRead - sent);
//     //         if (bWrite <= 0) break;
//     //         sent += bWrite;
//     //     }
//     // }
//     printf("Read: %lu\nWrite: %lu\n", bRead, bWrite);
//
//     close(file_fd);
//     return;
// }
