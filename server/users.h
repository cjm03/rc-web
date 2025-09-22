#ifndef USERS_H
#define USERS_H

#include <unistd.h>
#include <crypt.h>
#include <time.h>

#include "libbcrypt/bcrypt.h"

#define USERS_TABLE_SIZE 1024
#define SALT_SIZE 19
#define USER_PRIME1 10837
#define USER_PRIME2 11863

/////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////

typedef struct User {
    char* Username;
    char* Email;
    char PasswordHash[BCRYPT_HASHSIZE];
    struct User* next;
} User;

typedef struct UsersTable {
    int size;
    int count;
    User** users;
} UsersTable;

/////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////

int uiHash(const char* username, const int a, const int m);
int getUiHash(const char* username, const int size, const int attempt);
UsersTable* createNewUsersTable(void);
User* createNewUser(const char* username, const char* password, const char* email);
void insertUser(UsersTable* ut, const char* username, const char* password, const char* email);
User* userSearch(UsersTable* ut, const char* username);
void deleteUser(User* u);
int verifyPasswordHash(const char* password, const char* hashedPassword);

#endif // USERS_H
