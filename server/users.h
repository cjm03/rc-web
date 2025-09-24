#ifndef USERS_H
#define USERS_H

#include <unistd.h>
#include <crypt.h>
#include <time.h>

#include "libargon2/argon2.h"

#define USERS_TABLE_SIZE 32
#define HASHLEN 32
#define SALTLEN 16
#define ENCODEDLEN 128
#define MAXUSERNAME 32
#define MAXEMAIL 32

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
// User* createNewUser(const char* username, const char* password, const char* email);
void createInsertPopulateUser(UsersTable* ut, const char* username, const char* password, const char* email);
User* createEmptyUser(void);
User* userSearch(UsersTable* ut, const char* username);
void freeUser(User* u);
void freeUsersTable(UsersTable* ut);
void printUser(UsersTable* ut, const char* username);
int verifyPasswordHash(const char* password, const char* hashedPassword);
void testArgon2(uint32_t version, uint32_t t, uint32_t m, uint32_t p, char* pwd,
                char* salt, char* hexref, char* mcfref, argon2_type type);

#endif // USERS_H
