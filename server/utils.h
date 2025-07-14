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

/* logs client IP */
void logIP(const char* format, ...);

/*  */
SSL_CTX* initSSLCTX(void);

/*  */
void loadCerts(SSL_CTX* ctx, const char* certFile, const char* keyFile);

#endif // UTILS_H
