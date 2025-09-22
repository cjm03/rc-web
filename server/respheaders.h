#ifndef RESPHEADERS_H
#define RESPHEADERS_H

// Constants for simplicity when responding to requests
#define NOT_FOUND "HTTP/1.1 404 Not Found\r\n\r\n"
#define INTERNAL_ERROR "HTTP/1.1 500 Internal Server Error\r\n\r\n"
#define VID_OK "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\nContent-Type: video/mp4\r\nConnection: close\r\n\r\n"
#define TXT_OK "HTTP/1.1 200 OK\r\nContent-Length: %lld\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
#define CSS_OK "HTTP/1.1 200 OK\r\nContent-Length: %lld\r\nContent-Type: text/css\r\nSourceMap: %s\r\nConnection: close\r\n\r\n"
#define APP_OK "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n"
#define ICO_OK "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\nContent-Type: image/png\r\nConnection: close\r\n\r\n"

#endif // RESPHEADERS_H
