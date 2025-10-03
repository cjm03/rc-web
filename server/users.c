// users.c

// #define _DEFAULT_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <openssl/core.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/provider.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

#include "libargon2/argon2.h"
#include "users.h"

utemp* parseLogin(char* toparse)
{
    utemp* utmp = malloc(sizeof(utemp));
    utmp->u = malloc(sizeof(char) * MAXRAWCRED);
    utmp->p = malloc(sizeof(char) * MAXRAWCRED);
    char delim[2] = "&";
    char* token = strtok(toparse, delim);
    char* eq = strchr(token, '=');
    if (strlen(eq) >= 32) {
        fprintf(stderr, "POST data: username too long\n");
        free(utmp->u);
        free(utmp->p);
        free(utmp);
        exit(EXIT_FAILURE);
    }
    utmp->u = eq + 1;
    token = strtok(NULL, delim);
    eq = strchr(token, '=');
    if (strlen(eq) >= 32) {
        fprintf(stderr, "POST data: username too long\n");
        free(utmp->u);
        free(utmp->p);
        free(utmp);
        exit(EXIT_FAILURE);
    }
    utmp->p = eq + 1;
    return utmp;
}

// ##############
// #  Manager
// ##############

int uGenSalt(uint8_t* buffer, int bytes)
{
    int rand_ret = RAND_priv_bytes(buffer, bytes);
    if (rand_ret != 1) {

        fprintf(stderr, "RAND_priv_bytes: %lu\n", ERR_get_error());
        memset(buffer, 0x00, bytes);
        return -1;
    }
    return rand_ret;
}

void encodedHashArgon(char* password, char* encoded, uint8_t* salt)
{
    uint32_t pwdlen = strlen(password);
    uint8_t* pwd = (uint8_t*)strdup(password);
    uint32_t t_cost = 2;
    uint32_t m_cost = (1<<16);
    uint32_t parallelism = 1;

    int rc = argon2i_hash_encoded(t_cost, m_cost, parallelism,
                                  pwd, pwdlen, salt, SALTLEN, HASHLEN,
                                  encoded, ENCODEDLEN);
    free(pwd);
    if (ARGON2_OK != rc) {
        fprintf(stderr, "error: %s\n", argon2_error_message(rc));
    }
}

int loadStore(uTable* ut, const char* filepath)
{
    FILE* f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "loadStore: %s not accessed\n", filepath);
        return -1;
    }
    uint8_t nasalt[4] = "N/A";
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char delim[3] = "->";
    while ((read = getline(&line, &len, f)) != -1) {
        char* user;
        char* rawhash;
        user = strtok(line, delim);
        rawhash = strtok(NULL, delim);
        rawhash[strlen(rawhash) - 1] = '\0';
        insertUser(ut, user, nasalt, rawhash);
    }
    fclose(f);
    dumpTable(ut);
    return 1;
}

void storeNewUser(uTable* ut, const char* filename, char* username, char* password)
{
    FILE* f = fopen(filename, "a");
    uint8_t salt[SALTLEN];
    uGenSalt(salt, SALTLEN);
    char encoded[ENCODEDLEN];
    encodedHashArgon(password, encoded, salt);
    insertUser(ut, username, salt, encoded);
    fprintf(f, "%s->%s\n", username, encoded);
    fclose(f);
}

int loginAsUser(uTable* ut, char* username, char* password)
{
    User* u = getUser(ut, username);
    if (u == NULL) {
        fprintf(stderr, "user [%s] not found\n", username);
        return -1;
    }
    int rc = argon2i_verify(u->hash, password, strlen(password));
    if (ARGON2_OK != rc) {
        return -1;
    }
    return 1;
}

// ############
// #  Storage
// ############

int storageHash(const char* s, const int a, const int m)
{
    long hash = 0;
    const int len = strlen(s);
    for (int i = 0; i < len; i++) {
        hash +=(long)pow(a, len - (i + 1)) * s[i];
        hash = hash % m;
    }
    return (int)hash;
}

int storageGetHash(const char* s, const int users, const int attempt)
{
    const int hasha = storageHash(s, HT_PRIME1, users);
    const int hashb = storageHash(s, HT_PRIME2, users);
    return (hasha + (attempt * (hashb + 1))) % users;
}

uTable* createuTable(void)
{
    uTable* ut = malloc(sizeof(uTable));
    if (ut == NULL) {
        fprintf(stderr, "malloc ut failed\n");
        return NULL;
    }
    ut->capacity = MAXUSERS;
    ut->count = 0;
    ut->users = calloc(ut->capacity, sizeof(User*));
    if (ut->users == NULL) {
        fprintf(stderr, "calloc ut->users failed\n");
        free(ut);
        return NULL;
    }
    return ut;
}

User* createUser(char* username, uint8_t* salt, char* hash)
{
    User* u = malloc(sizeof(User));
    if (u == NULL) {
        fprintf(stderr, "malloc u failed\n");
        return NULL;
    }
    u->username = strdup(username);
    u->salt = malloc(sizeof(uint8_t) * SALTLEN);
    u->hash = malloc(sizeof(char) * ENCODEDLEN);
    memcpy(u->salt, salt, SALTLEN);
    memcpy(u->hash, hash, ENCODEDLEN);
    return u;
}

void insertUser(uTable* ut, char* username, uint8_t* salt, char* hash)
{
    User* new = createUser(username, salt, hash);
    int index = storageGetHash(new->username, ut->capacity, 0);
    User* cur = ut->users[index];
    int i = 1;
    while (cur != NULL) {
        if (strcmp(cur->username, username) == 0) {
            destroyUser(cur);
            ut->users[index] = new;
            return;
        }
        index = storageGetHash(new->username, ut->capacity, i);
        cur = ut->users[index];
        i++;
    }
    ut->users[index] = new;
    ut->count++;
}

User* getUser(uTable* ut, const char* username)
{
    int index = storageGetHash(username, ut->capacity, 0);
    User* cur = ut->users[index];
    int i = 1;
    while (cur != NULL) {
        if (strcmp(cur->username, username) == 0) {
            return cur;
        }
        index = storageGetHash(username, ut->capacity, i);
        cur = ut->users[index];
        i++;
    }
    return NULL;
}

void dumpTable(uTable* ut)
{
    for (size_t i = 0; i < ut->capacity; ++i) {
        User* cur = ut->users[i];
        if (cur != NULL) {
            printf("[%s]:%s -> %s\n", cur->username, cur->salt, cur->hash);
        }
    }
}

void destroyTable(uTable* ut)
{
    for (size_t i = 0; i < ut->capacity; ++i) {
        User* cur = ut->users[i];
        if (cur != NULL) {
            destroyUser(ut->users[i]);
        }
    }
    free(ut->users);
    free(ut);
}

void destroyUser(User* u)
{
    free(u->username);
    free(u->salt);
    free(u->hash);
    free(u);
}







