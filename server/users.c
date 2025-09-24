// users.c

// #define _DEFAULT_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <openssl/rand.h>

#include "libargon2/argon2.h"

#include "users.h"
// #include "utils.h"


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

void createInsertPopulateUser(UsersTable* ut, const char* username, const char* password, const char* email)
{
    User* u = createEmptyUser();

    strcpy(u->username, username);
    strcpy(u->email, email);

    unsigned char out[HASHLEN];
    unsigned char hexout[HASHLEN * 2 + 4];
    char enc[ENCODEDLEN];
    int ret, i;
    uint8_t salt[SALTLEN] = {0x02, 0x04, 0x07, 0x11, 0x10, 0xf2, 0x9a, 0x44,
                             0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0xdd, 0xbb};
    uint32_t t_cost = 2;
    uint32_t m_cost = (1 << 16);
    uint32_t parallelism = 1;

    printf("Testing: $v=%d t=%d, m=%d, p=%d, pass=%s, salt=%s: ",
           ARGON2_VERSION_13, t_cost, m_cost, parallelism, password, salt);


    // uint8_t* pwd = (uint8_t*)strdup(password);
    // uint32_t pwdlen = strlen((char*)pwd);


    // argon2i_hash_raw(t_cost, m_cost, parallelism, password, strlen(password), salt, SALTLEN, out, HASHLEN, enc, ENCODEDLEN);
    ret = argon2_hash(t_cost, m_cost, parallelism, password, strlen(password), salt, SALTLEN, out, HASHLEN, enc, ENCODEDLEN, Argon2_i, ARGON2_VERSION_13);

    // argon2_context context = {
    //     out, 
    //     HASHLEN, 
    //     pwd, 
    //     strlen(password), 
    //     salt, 
    //     SALTLEN, 
    //     NULL, 0, 
    //     NULL, 0, 
    //     t_cost, m_cost, parallelism, parallelism,
    //     ARGON2_VERSION_13, 
    //     NULL, NULL, 
    //     ARGON2_DEFAULT_FLAGS
    // };

    // int rc = argon2i_ctx(&context);
    if (ARGON2_OK != ret) {
        printf("Error: %s\n", argon2_error_message(ret));
        freeUser(u);
        exit(1);
    }

    // free(pwd);

    for(i = 0; i < HASHLEN; ++i ) {
        sprintf((char*)(hexout + i * 2), "%02x", out[i] );
        printf( "\n" );
    }

    // if (memcmp(hash1, hash2, HASHLEN)) {
    //     for (int i = 0; i < HASHLEN; ++i) {
    //         printf("%02x", hash2[i]);
    //     }
    //     printf("hash failure, bailing\n");
    //     freeUser(u);
    //     exit(1);
    // }

    ret = argon2_verify(enc, password, strlen(password), Argon2_i);
    // int verify = argon2i_verify_ctx(&context, (char*)hash2);
    if (ret != ARGON2_OK) {
        printf("ctx verification failure, bailing\n");
        freeUser(u);
        exit(1);
    }

    // u->hash = hash2;
    u->salt = salt;
}

User* createEmptyUser(void)
{
    User* u = malloc(sizeof(User));
    if (!u) {
        fprintf(stderr, "createEmptyUser: User* u [malloc] error\n");
        return NULL;
    }
    u->username = malloc(MAXUSERNAME * sizeof(char));
    u->email = malloc(MAXEMAIL * sizeof(char));
    u->hash = malloc(HASHLEN * sizeof(uint8_t));
    u->salt = malloc(SALTLEN * sizeof(uint8_t));
    return u;
}

User* userSearch(UsersTable* ut, const char* username);

void freeUser(User* u)
{
    free(u->username);
    free(u->email);
    free(u->hash);
    free(u->salt);
    free(u);
}

void freeUsersTable(UsersTable* ut)
{
    for (int i = 0; i < ut->size; ++i) {
        User* cur = &ut->users[i];
        if (cur != NULL) freeUser(cur);
    }
    free(ut->users);
    free(ut);
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

void
testArgon2(uint32_t version, uint32_t t, uint32_t m, uint32_t p,
           char* pwd, char* salt, char* hexref, char* mcfref, argon2_type type)
{
    unsigned char out[HASHLEN];
    unsigned char hexout[HASHLEN * 2 + 4];
    char encoded[ENCODEDLEN];
    int ret, i;

    printf("Testing: $v=%d t=%d, m=%d, p=%d, pass=%s, salt=%s: ",
           version, t, m, p, pwd, salt);

    ret = argon2_hash(t, 1 << m, p, pwd, strlen(pwd), salt, strlen(salt), out, HASHLEN,
                      encoded, ENCODEDLEN, type, version);
    if (ret != ARGON2_OK) {
        printf("error: argon2_hash, bad return val\n");
        exit(1);
    }
    for (i = 0; i < HASHLEN; ++i) {
        sprintf((char*)(hexout + i * 2), "%02x", out[i]);
    }
    printf("%s\n", hexout);
    if (memcmp(hexout, hexref, HASHLEN * 2) != 0) {
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


/////////////////////////////////
// Save & Load to File
/////////////////////////////////

void saveUserInfoToFile(UsersTable* ut, const char* filename);

void loadUserInfoFromFile(const char* filename);

int main(void)
{
    int ret;
    unsigned char* randsalt = malloc(sizeof(char) * 16);
    unsigned char out[HASHLEN];
    char const* msg;
    int version = ARGON2_VERSION_13;
    RAND_priv_bytes(randsalt, sizeof(char) * 16);
    printf("\nRANDSALT:%s ]end\n\n", randsalt);

    testArgon2(version, 2, 16, 1, "bbvheysPFqnuAg1", "whatdoesthefoxdo",
               "cc47159dc2cbdb4e050f57dbb3ca7e94f478606fca00c65c75c08403ef3585e3",
               "$argon2i$v=19$m=65536,t=2,p=1$d2hhdGRvZXN0aGVmb3hkbw"
               "$zEcVncLL204FD1fbs8p+lPR4YG/KAMZcdcCEA+81heM", Argon2_i);

    // testArgon2(version, 2, 16, 1, "bbvheysPFqnuAg1", "whatdoesthefoxdo",
    //            "31ea05320865cbdb93a36f8b823bbb14a8df6aa94725fd989e8eef5d2039135c",
    //            "$argon2i$v=19$m=16,t=2,p=1$d2hhdGRvZXN0aGVmb3hkbw"
    //            "$MeoFMghly9uTo2+Lgju7FKjfaqlHJf2Yno7vXSA5E1w", Argon2_i);

    // const char username[] = "cjmoye";
    // const char password[] = "bbvheysPFqnuAg1";
    // const char email[] = "cjmoye@iu.edu";
    // UsersTable* ut = createNewUsersTableSized(4);
    // createInsertPopulateUser(ut, "cjmoye", "bbvheysPFqnuAg1", "cjmoye@iu.edu");
    // createInsertPopulateUser(ut, username, password, email);
    // printUser(ut, username);
    // freeUsersTable(ut);
}







