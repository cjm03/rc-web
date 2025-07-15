// users.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "users.h"
#include "utils.h"
#include "libbcrypt/bcrypt.h"

static User DELETEDUSER = {NULL, NULL, {0}, {0}, NULL};

// hash

int uiHash(const char* username, const int a, const int m)
{
    long hash = 0;
    int len = strlen(username);
    for (int i = 0; i < len; i++) {
        hash += (long)pow(a, len - (i + 1)) * username[i];
        hash = hash % m;
    }
    return (int)hash;
}

int getUiHash(const char* username, const int size, const int attempt)
{
    const int hasha = uiHash(username, USER_PRIME1, size);
    const int hashb = uiHash(username, USER_PRIME2, size);
    return (hasha + (attempt * (hashb + 1))) % size;
}

// create

UsersTable* createNewUsersTable(void)
{
    UsersTable* ut = malloc(sizeof(UsersTable));
    ut->size = USERS_TABLE_SIZE;
    ut->count = 0;
    ut->users = calloc(ut->size, sizeof(User*));
    return ut;
}

User* createNewUser(const char* username, const char* password, const char* email)
{
    int ret;
    User* NewUser = malloc(sizeof(User));
    NewUser->Username = strdup(username);
    NewUser->Email = strdup(email);

    ret = bcrypt_gensalt(12, NewUser->Salt);
    assert(ret == 0);
    ret = bcrypt_hashpw(password, NewUser->Salt, NewUser->PasswordHash);
    assert(ret == 0);

    if (verifyPasswordHash(password, NewUser->PasswordHash) == 1) {
        fprintf(stderr, "password matches\n");
        return NewUser;
        // int index = uiHash(NewUser->Username, USER_PRIME1, USERS_TABLE_SIZE);
        // if (ut->users[index] == NULL) {
        //     ut->users[index] = NewUser;
        // } else {
        //     printf("INSERTION FAILED\n");
        //     free(NewUser);
        //     return NULL;
        // }
    } else {
        printf("PASSWORD MATCH FAILED\n");
        free(NewUser);
        return NULL;
    }
    // printf("creatednewuser\n");
    // return NewUser;
}

void insertUser(UsersTable* ut, const char* username, const char* password, const char* email)
{
    User* new = createNewUser(username, password, email);
    printf("%ld\n", strlen(new->PasswordHash));
    int index = getUiHash(new->Username, ut->size, 0);
    User* cur = ut->users[index];
    int i = 1;
    while (cur != NULL) {
        if (cur != &DELETEDUSER) {
            if (strcmp(cur->Username, username) == 0) {
                deleteUser(cur);
                ut->users[index] = new;
                return;
            }
        }
        index = getUiHash(new->Username, ut->size, i);
        cur = ut->users[index];
        i++;
    }
    ut->users[index] = new;
    ut->count++;
    printf("[%s] inserted\n", username);
    return;
}

User* userSearch(UsersTable* ut, const char* username)
{
    int index = getUiHash(username, ut->size, 0);
    User* user = ut->users[index];
    int i = 1;
    while (user != NULL) {
        if (user != &DELETEDUSER) {
            if (strcmp(user->Username, username) == 0) {
                return user;
            }
        }
        index = getUiHash(username, ut->size, i);
        user = ut->users[index];
        i++;
    }
    return NULL;
}

void deleteUser(User* u)
{
    free(u->Username);
    free(u->Email);
    free(u);
}

int verifyPasswordHash(const char* password, const char* hashedPassword)
{
    int ret;

    ret = bcrypt_checkpw(password, hashedPassword);
    assert(ret != -1);

    if (ret == 0) {
        return 1;
    } else {
        return 0;
    }
}

/////////////////////////////////
// Save & Load to File
/////////////////////////////////

void saveUserInfoToFile(UsersTable* ut, const char* filename)
{
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("fopen");
        return;
    }

    for (int i = 0; i < USERS_TABLE_SIZE; i++) {
        User* u = ut->users[i];
        while (u) {
            for (int j = 0; j < SALT_SIZE; j++) {
                fprintf(file, "%02x", u->Salt[j]);
            }
            fprintf(file, ",");

            for (int k = 0; k < BCRYPT_HASHSIZE; k++) {
                fprintf(file, "%02x", u->PasswordHash[k]);
            }
            fprintf(file, ",%s,%s\n", u->Username, u->Email);

            u = u->next;
        }
    }
    fclose(file);
}

void loadUserInfoFromFile(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return;
    }

    char saltHex[SALT_SIZE * 2 + 1];
    char hashHex[BCRYPT_HASHSIZE * 2 + 1];
    char username[256];
    char email[256];

    while (fscanf(file, "%38s,%128s,%255[^,],%255s\n", saltHex, hashHex, username, email) == 4) {
        User* new = malloc(sizeof(User));
        new->Username = strdup(username);
        new->Email = strdup(email);
        hexStringToBytes(saltHex, (unsigned char*)new->Salt, SALT_SIZE);
        hexStringToBytes(hashHex, (unsigned char*)new->PasswordHash, BCRYPT_HASHSIZE);

        // TODO: Hash Username for index, then
        // `new->next = UsersTable[index];`
        // `UsersTable[index] = new;`
    }
    fclose(file);
}








