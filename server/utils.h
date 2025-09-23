#ifndef UTILS_H
#define UTILS_H

// #define _DEFAULT_SOURCE
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

void StrToHex(const char* in, uint8_t* out, size_t length);

void SaltGen(void);

/* logs client IP */
void logIP(const char* format, ...);

/*  */
SSL_CTX* initSSLCTX(void);

/*  */
void loadCerts(SSL_CTX* ctx, const char* certFile, const char* keyFile);

/*  */
int findHeaderEnd(const char* buf, int len);
int extractContentLength(const char* headers);
char* readFullRequest(SSL* ssl, int* outlen);

#endif // UTILS_H
