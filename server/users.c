// users.c

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "users.h"
#include "libargon2/argon2.h"

// create

UsersTable* createNewUsersTable(void)
{
    return createNewUsersTableSized(USERS_TABLE_SIZE);
}

UsersTable* createNewUsersTableSized(const int size)
{
    UsersTable* ut = malloc(sizeof(UsersTable));
    ut->size = size;
    ut->count = 0;
    ut->users = calloc(ut->size, sizeof(User*));
    return ut;
}

User* createNewUser(const char* username, const char* password, const char* email)
{
    User* u = malloc(sizeof(User));
    u->username = strdup(username);
    u->email = strdup(email);
    uint8_t hash1[HASHLEN];
    uint8_t hash2[HASHLEN];

    uint8_t salt[SALTLEN];
    memset(salt, 0x00, SALTLEN);
    // uint8_t salt[] = {0};
    //
    // for (int i = 0; i < SALTLEN; i++) {
    //     salt[i] = (uint8_t)(rand() % 255);
    // }
    // printf("%s\n", salt);

    printf("pwd: %s\n", password);
    uint8_t* pwd = (uint8_t*)strdup(password);
    printf("%s\n", pwd);
    uint32_t pwdlen = strlen((char*)pwd);
    uint32_t t_cost = 2;
    uint32_t m_cost = (1 << 16);
    uint32_t parallelism = 1;
    argon2i_hash_raw(t_cost, m_cost, parallelism, pwd, pwdlen, salt, SALTLEN, hash1, HASHLEN);
    argon2_context ctx = {
        hash2, HASHLEN, pwd, pwdlen, salt, SALTLEN, NULL, 0, NULL, 0, t_cost, m_cost, parallelism, parallelism,
        ARGON2_VERSION_13, NULL, NULL, ARGON2_DEFAULT_FLAGS
    };
    int rc = argon2i_ctx(&ctx);
    if (ARGON2_OK != rc) {
        printf("Error: %s\n", argon2_error_message(rc));
        exit(1);
    }
    free(pwd);
    printf("hash1: ");
    for( int i=0; i<HASHLEN; ++i ) printf( "%02x", hash1[i] ); printf( "\nhash2: " );
    for( int i=0; i<HASHLEN; ++i ) printf( "%02x", hash2[i] ); printf( "\n" );
    if (memcmp(hash1, hash2, HASHLEN)) {
        for (int i = 0; i < HASHLEN; ++i) {
            printf("%02x", hash2[i]);
        }
        printf("hash failure, bailing\n");
        freeUser(u);
        exit(1);
    }
    int verify = argon2i_verify_ctx(&ctx, hash2);
    if (verify != 0) {
        printf("ctx verification failure, bailing\n");
        freeUser(u);
        exit(1);
    }
    printf("%zu %zu\n", strlen(hash1), strlen(hash2));
    u->hash = hash2;
    StrToHex(salt, u->salt, SALTLEN);
    return u;
}

void insertUser(UsersTable* ut, const char* username, const char* password, const char* email)
{
    User* new = createNewUser(username, password, email);
    ut->users[ut->count] = *new;
    ut->count++;
}

User* userSearch(UsersTable* ut, const char* username);

void freeUser(User* u)
{
    free(u->username);
    free(u->email);
    free(u);
}

void printUser(UsersTable* ut, const char* username)
{
    for (int i = 0; i < ut->size; i++) {
        User cur = ut->users[i];
        if (strncmp(cur.username, username, strlen(username)) == 0) {
            printf("User %d\n%s\n%s\n%s\n%s\n", i, cur.username, cur.email, cur.hash, cur.salt);
            return;
        }
    }
    printf("not found\n");
}

int verifyPasswordHash(const char* password, const char* hashedPassword);

void StrToHex(const char* in, uint8_t *out, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        out[i] = (uint8_t)in[i];
    }
}
/////////////////////////////////
// Save & Load to File
/////////////////////////////////

void saveUserInfoToFile(UsersTable* ut, const char* filename);

void loadUserInfoFromFile(const char* filename);

int main(void)
{
    const char* username = "cjmoye";
    const char* password = "bbvheysPFqnuAg1";
    const char* email = "cjmoye@iu.edu";
    UsersTable* ut = createNewUsersTable();
    insertUser(ut, username, password, email);
    printUser(ut, username);
}







