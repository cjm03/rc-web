#ifndef UTILS_H
#define UTILS_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

//===================================
// functions
//===================================

/* decodes a string with url encoding */
void urldecode(char* dest, const char* source);

/* string of hexadecimal to bytes */
void hexStringToBytes(const char* hexstr, unsigned char* buffer, size_t len);

/* logs client IP */
void logIP(const char* format, ...);

/*  */
SSL_CTX* initSSLCTX(void);

/*  */
void loadCerts(SSL_CTX* ctx, const char* certFile, const char* keyFile);

#endif // UTILS_H
