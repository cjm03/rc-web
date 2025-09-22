#ifndef USERS_H
#define USERS_H

#include <unistd.h>
#include <crypt.h>
#include <time.h>

#include "libargon2/argon2.h"

#define USERS_TABLE_SIZE 32
#define HASHLEN 32
#define SALTLEN 16
#define USER_PRIME1 10837
#define USER_PRIME2 11863

/////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////

typedef struct User {
    char* username;
    char* email;
    uint8_t* hash;
    uint8_t* salt;
    // char hash[HASHLEN];
    // char salt[SALTLEN]
} User;

typedef struct UsersTable {
    int size;
    int count;
    User* users;
} UsersTable;

/////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////

UsersTable* createNewUsersTable(void);
UsersTable* createNewUsersTableSized(const int size);
User* createNewUser(const char* username, const char* password, const char* email);
void insertUser(UsersTable* ut, const char* username, const char* password, const char* email);
User* userSearch(UsersTable* ut, const char* username);
void freeUser(User* u);
int verifyPasswordHash(const char* password, const char* hashedPassword);
void StrToHex(const char* in, uint8_t* out, size_t length);

#endif // USERS_H
