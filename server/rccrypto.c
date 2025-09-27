// rccrypto.c


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <openssl/core.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/provider.h>

#include "libargon2/argon2.h"

#include "rccrypto.h"



void
testArgon2(uint32_t version, uint32_t t, uint32_t m, uint32_t p,
           char* pwd, char* salt, char* hexref, char* mcfref, argon2_type type)
{
    unsigned char out[T_ARGON2_HASHLEN];
    unsigned char hexout[T_ARGON2_HASHLEN * 2 + 4];
    char encoded[T_ARGON2_ENCODEDLEN];
    int ret, i;

    printf("Testing: $v=%d t=%d, m=%d, p=%d, pass=%s, salt=%s: ",
           version, t, m, p, pwd, salt);

    ret = argon2_hash(t, 1 << m, p, pwd, strlen(pwd), salt, strlen(salt), out, T_ARGON2_HASHLEN,
                      encoded, T_ARGON2_ENCODEDLEN, type, version);
    if (ret != ARGON2_OK) {
        printf("error: argon2_hash, bad return val\n");
        exit(1);
    }
    for (i = 0; i < T_ARGON2_HASHLEN; ++i) {
        sprintf((char*)(hexout + i * 2), "%02x", out[i]);
    }
    printf("%s\n", hexout);
    if (memcmp(hexout, hexref, T_ARGON2_HASHLEN * 2) != 0) {
        printf("error: hexes dont match\n");
        exit(1);
    }
    if (memcmp(encoded, mcfref, strlen(mcfref)) != 0) {
        printf("error: mismatch encoded <-> mcfref\n");
        exit(1);
    }
    ret = argon2_verify(encoded, pwd, strlen(pwd), type);
    if (ret == ARGON2_OK) {
        printf("Encoded <-> Password: Clean\n");
    } else {
        printf("Encoded <-> Password: mismatch\n");
        exit(1);
    }
    ret = argon2_verify(mcfref, pwd, strlen(pwd), type);
    if (ret == ARGON2_OK) {
        printf("mcfref <-> Password: Clean\n");
    } else {
        printf("mcfref <-> Password: mismatch\n");
        exit(1);
    }
    printf("PASS\n");
}

uint8_t* privateGeneration(void)
{
    OSSL_LIB_CTX* libctx = OSSL_LIB_CTX_new();
    OSSL_LIB_CTX_set0_default(libctx);
}


int main(void)
{
    int ret;
    unsigned char* xrandsalt = malloc(sizeof(char) * 16);
    uint8_t yrandsalt[T_ARGON2_SALTLEN];
    unsigned char out[T_ARGON2_HASHLEN];
    char const* msg;
    int version = ARGON2_VERSION_13;
    RAND_priv_bytes(xrandsalt, sizeof(char) * 16);
    RAND_priv_bytes(yrandsalt, sizeof(uint8_t) * T_ARGON2_SALTLEN);
    printf("\nxRANDSALT:%s ]end\n\n", xrandsalt);
    printf("\nyRANDSALT:%s ]end\n\n", yrandsalt);

    testArgon2(version, 2, 16, 1, "bbvheysPFqnuAg1", "whatdoesthefoxdo",
               "cc47159dc2cbdb4e050f57dbb3ca7e94f478606fca00c65c75c08403ef3585e3",
               "$argon2i$v=19$m=65536,t=2,p=1$d2hhdGRvZXN0aGVmb3hkbw"
               "$zEcVncLL204FD1fbs8p+lPR4YG/KAMZcdcCEA+81heM", Argon2_i);
    free(xrandsalt);
    return 0;
}







