#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void POLYCLAMP(uint8_t r[16])
{
    r[3] &= 15;
    r[7] &= 15;
    r[11] &= 15;
    r[15] &= 15;
    r[4] &= 252;
    r[8] &= 252;
    r[12] &= 252;
}

void StrToHex(const char* in, uint8_t* out, size_t length)
{
    for (size_t i = 0; i < length; i++) {
        out[i] = (uint8_t)in[i];
    }
}

uint64_t U8TO64(uint8_t arr[])
{
    return (((uint64_t)(arr[0] & 0xff)      ) |
            ((uint64_t)(arr[1] & 0xff) <<  8) |
            ((uint64_t)(arr[2] & 0xff) << 16) |
            ((uint64_t)(arr[3] & 0xff) << 24) |
            ((uint64_t)(arr[4] & 0xff) << 32) |
            ((uint64_t)(arr[5] & 0xff) << 40) |
            ((uint64_t)(arr[6] & 0xff) << 48) |
            ((uint64_t)(arr[7] & 0xff) << 56));
}

int main(void)
{
    // uint8_t key[32] = {
    //     0x10abcdef, 0x20abcdef, 0x30abcdef, 0x40abcdef, 0x50abcdef, 0x60abcdef, 0x70abcdef, 0x80abcdef,
    //     0xa0abcdef, 0xb0abcdef, 0xc0abcdef, 0xd0abcdef, 0xe0abcdef, 0xf0abcdef, 0x1aabcdef, 0x1babcdef,
    //     0x1cabcdef, 0x1dabcdef, 0x1eabcdef, 0x1fabcdef, 0xc1abcdef, 0xd1abcdef, 0xe1abcdef, 0xf1abcdef,
    //     0x90abcdef, 0x91abcdef, 0x92abcdef, 0x93abcdef, 0x94abcdef, 0x95abcdef, 0x96abcdef, 0x97abcdef
    // };
    uint8_t key[32] = {
        0x85, 0xd6, 0x30, 0x40,
        0x50, 0x60, 0x70, 0x80,
        0xa0, 0xb0, 0xc0, 0xd0,
        0xe0, 0xf0, 0x1a, 0x1b,
        0x1c, 0x1d, 0x1e, 0x1f,
        0xc1, 0xd1, 0xe1, 0xf1,
        0x90, 0x91, 0x92, 0x93,
        0x94, 0x95, 0x96, 0x97
    };
    uint8_t r[16];
    uint8_t s[16];
    uint64_t a = 0;
    const char* msg = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
    uint8_t msgarr[strlen(msg)];
    StrToHex(msg, msgarr, strlen(msg));
    int smallblock = strlen(msg) % 16;
    int blocks = (strlen(msg) - smallblock) / 16;
    for (size_t i = 0; i < blocks + 1; i++) {
        int x = i * 16;
        uint64_t top = U8TO64(&msgarr[x]);
        uint64_t bottom = U8TO64(&msgarr[x + 8]);
        r[0] = ( top                        ) & 0xffc0fffffff;
        r[1] = ((top >> 44) | (bottom << 20)) & 0xfffffc0ffff;
        r[2] = ((bottom >> 24)              ) & 0x00ffffffc0f;
    }
}
