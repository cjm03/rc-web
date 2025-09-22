#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
// #include <byteswap.h>
#include <math.h>
#include "chacha.h"

//-------------------------------------------------------------------------------------

void PRINTBLOCK(uint32_t *state)
{
    printf("0x%" PRIXPTR " 0x%" PRIXPTR " 0x%" PRIXPTR " 0x%" PRIXPTR "\n",   (uintptr_t)state[0],  (uintptr_t)state[1],  (uintptr_t)state[2],  (uintptr_t)state[3]);
    printf("0x%" PRIXPTR " 0x%" PRIXPTR " 0x%" PRIXPTR " 0x%" PRIXPTR "\n",   (uintptr_t)state[4],  (uintptr_t)state[5],  (uintptr_t)state[6],  (uintptr_t)state[7]);
    printf("0x%" PRIXPTR " 0x%" PRIXPTR " 0x%" PRIXPTR " 0x%" PRIXPTR "\n",   (uintptr_t)state[8],  (uintptr_t)state[9],  (uintptr_t)state[10], (uintptr_t)state[11]);
    printf("0x%" PRIXPTR " 0x%" PRIXPTR " 0x%" PRIXPTR " 0x%" PRIXPTR "\n\n", (uintptr_t)state[12], (uintptr_t)state[13], (uintptr_t)state[14], (uintptr_t)state[15]);
}

// apply the quarterround function on array passed as `out`
void QUARTERROUND(uint32_t* out, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    out[a] += out[b];
    out[d] ^= out[a];
    out[d] = lrot32(out[d], 16);

    out[c] += out[d];
    out[b] ^= out[c];
    out[b] = lrot32(out[b], 12);

    out[a] += out[b];
    out[d] ^= out[a];
    out[d] = lrot32(out[d], 8);

    out[c] += out[d];
    out[b] ^= out[c];
    out[b] = lrot32(out[b], 7);
}

uint32_t lrot32(uint32_t n, unsigned int d) {
    return (n << d) | (n >> (INT_BITS - d));
}
uint32_t rrot32(uint32_t n, unsigned int d) {
    return lrot32(n, 32 - (d));
}
static uint32_t PACK4(const uint8_t* a) {
    uint32_t res = (uint32_t)a[0] << 0 * 8 
        | (uint32_t)a[1] << 1 * 8 
        | (uint32_t)a[2] << 2 * 8 
        | (uint32_t)a[3] << 3 * 8;
    return res;
}

void CHACHA20_INIT(Context* Context, uint8_t *key, uint8_t *nonce)
{
    memcpy(Context->key, key, sizeof(Context->key));
    memcpy(Context->nonce, nonce, sizeof(Context->nonce));

    Context->state[0]  = PACK4((const uint8_t*)CHACHA20_MAGICSTRING + 0 * 4);
    Context->state[1]  = PACK4((const uint8_t*)CHACHA20_MAGICSTRING + 1 * 4);
    Context->state[2]  = PACK4((const uint8_t*)CHACHA20_MAGICSTRING + 2 * 4);
    Context->state[3]  = PACK4((const uint8_t*)CHACHA20_MAGICSTRING + 3 * 4);
    Context->state[4]  = PACK4(key + 0 * 4);
    Context->state[5]  = PACK4(key + 1 * 4);
    Context->state[6]  = PACK4(key + 2 * 4);
    Context->state[7]  = PACK4(key + 3 * 4);
    Context->state[8]  = PACK4(key + 4 * 4);
    Context->state[9]  = PACK4(key + 5 * 4);
    Context->state[10] = PACK4(key + 6 * 4);
    Context->state[11] = PACK4(key + 7 * 4);
    Context->state[12] = 0;
    Context->state[13] = PACK4(nonce + 0 * 4);
    Context->state[14] = PACK4(nonce + 1 * 4);
    Context->state[15] = PACK4(nonce + 2 * 4);

    Context->index = 0;
}

void CHACHA20_BLOCK(const uint32_t in[16], uint32_t out[16])
{
    for (int i = 0; i < 16; i++) {
        out[i] = in[i];
    }

    for (int i = 0; i < 10; i++) {
        QUARTERROUND(out,  0,  4,  8, 12);      // 1
        QUARTERROUND(out,  1,  5,  9, 13);      // 2
        QUARTERROUND(out,  2,  6, 10, 14);      // 3
        QUARTERROUND(out,  3,  7, 11, 15);      // 4
        QUARTERROUND(out,  0,  5, 10, 15);      // 5
        QUARTERROUND(out,  1,  6, 11, 12);      // 6
        QUARTERROUND(out,  2,  7,  8, 13);      // 7
        QUARTERROUND(out,  3,  4,  9, 14);      // 8
    }

    for (int i = 0; i < 16; i++) {
        out[i] += in[i];
    }
}

void CHACHA20_CONTEXT_INIT(Context *Context, uint8_t key[], uint8_t nonce[], uint32_t counter, unsigned long ptlen)
{
    memset(Context, 0, sizeof(struct Context));
    CHACHA20_INIT(Context, key, nonce);
    Context->state[12] = counter;
    Context->counter = counter;
    uint8_t* tmp = malloc(ptlen);
    if (tmp == NULL) {
        printf("ctx init malloc\n");
        exit(EXIT_FAILURE);
    }
    Context->keystream = tmp;
}

// count: tells serialize how to store the transformed state in the keystream
//      1: [0]->[63]    2: [64]->[127] ...
// Applies to a block following the block operation
void CHACHA20_SERIALIZE(uint32_t* state, uint8_t* keystream, unsigned long index, unsigned long size)
{
    unsigned int offset = index;
    unsigned int bytes = (size > 64) ? 64 : size;
    for (unsigned int i = 0; i < bytes; i += 4) {
        uint32_t cur = state[i / 4];
        if (offset + i + 0 == size) break;
        keystream[offset + i + 0] = (cur >> 0) & 0xff;                 
        if (offset + i + 1 == size) break;
        keystream[offset + i + 1] = (cur >> 8) & 0xff;     
        if (offset + i + 2 == size) break;
        keystream[offset + i + 2] = (cur >> 16) & 0xff;    
        if (offset + i + 3 == size) break;
        keystream[offset + i + 3] = (cur >> 24) & 0xff;     
    }
}

// print a certain amount the keystream
void PRINTSERIALIZED(uint8_t* keystream, size_t size)
{
    for (size_t i = 0; i < size; i += 16) {
        printf("  %03zx  ", i);

        // hex
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                printf("%02x ", keystream[i + j]);
            } else {
                printf("   ");
            }
        }
        printf(" ");
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                char c = keystream[i + j];
                printf("%c", isprint((unsigned char)c) ? c : '.');
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Performs chacha20 on the rest of the plaintext if the initial keystream isnt enough
void CHACHA20_XOR(Context* Context, uint8_t* plaintext, unsigned long pt_size)
{
    // PRINTSERIALIZED(plaintext, pt_size);
    unsigned long size_ceiled = ceil((float)pt_size / 64);
    for (int i = 0; i < size_ceiled; i++) {
        uint32_t state[16];
        CHACHA20_BLOCK(Context->state, state);
        CHACHA20_SERIALIZE(state, Context->keystream, Context->index, pt_size);
        Context->index += 64;
        Context->state[12]++;
    }
    // PRINTSERIALIZED(Context->keystream, pt_size);
    uint8_t* tmp = malloc(pt_size);
    if (tmp == NULL) {
        printf("XOR malloc error\n");
        exit(EXIT_FAILURE);
    }
    Context->buffer = tmp;
    for (int y = 0; y < pt_size; y++) {
        Context->buffer[y] = plaintext[y] ^ Context->keystream[y];
    }
}

void StrToHex(const char* in, uint8_t *out, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        out[i] = (uint8_t)in[i];
    }
}

void CHACHA20_DECRYPT(Context* Context, uint8_t* ciphertext, unsigned long pt_size)
{
    for (int i = 0; i < pt_size; i++) {
        Context->buffer[i] = ciphertext[i] ^ Context->keystream[i];
    }

}

// CURRENT TESTING IMPLEMENTATION. 
// int main(void)
// {
//     uint8_t key[32] = {
//         0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
//         0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
//         0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
//         0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
//     };
//     uint8_t nonce[12] = {
//         0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00
//     };
//     uint32_t counter = 0;
//     Context Context;
//     const char* msg = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
//     uint8_t data[strlen(msg)];
//     StrToHex(msg, data, strlen(msg));
//     PRINTSERIALIZED(data, sizeof(data));
//     CHACHA20_CONTEXT_INIT(&Context, key, nonce, counter, sizeof(data)); // CHACHA20_CONTEXT_INIT(&Context, key, nonce, counter, strlen(msg));
//     CHACHA20_XOR(&Context, data, sizeof(data)); // CHACHA20_XOR(&Context, data, strlen(msg));
//     PRINTSERIALIZED(Context.buffer, sizeof(data));
//
//     CHACHA20_DECRYPT(&Context, Context.buffer, sizeof(data));
//     PRINTSERIALIZED(Context.buffer, sizeof(data));
//
//     free(Context.keystream);
//     free(Context.buffer);
// }

