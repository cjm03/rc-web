#include "chacha.c"
#include <stdint.h>

int main(void)
{
    Context Context;

    // 256-bit KEY
    uint8_t key[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };

    // 96-bit NONCE
    uint8_t nonce[12] = {
        0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00
    };
    
    // 32-bit COUNTER
    uint32_t counter = 0;

    // variable length message to encrypt, converted to uint8_t array w/ StrToHex()
    const char* msg = "Despite the intense settings that we produce throughout the duration of the cat in the 9th life, there are few and far between minutes regarding; Four score and 7even years ago; $&!()(a despite/without) Ladies and gentlemen!";
    uint8_t data[strlen(msg)];
    StrToHex(msg, data, strlen(msg));

    // Print the message prior to encryption
    PRINTSERIALIZED(data, sizeof(data));

    // Initialize the ChaCha20 context
    CHACHA20_CONTEXT_INIT(&Context, key, nonce, counter, sizeof(data));

    // Perform the ChaCha20 algorithm
    CHACHA20_XOR(&Context, data, sizeof(data));

    // Print the encrypted message
    PRINTSERIALIZED(Context.buffer, sizeof(data));

    // Decrypt the encrypted message
    CHACHA20_DECRYPT(&Context, Context.buffer, sizeof(data));

    // Print the decrypted message
    PRINTSERIALIZED(Context.buffer, sizeof(data));

    // free allocations made to the heap
    free(Context.keystream);
    free(Context.buffer);
}
