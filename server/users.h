#ifndef USERS_H
#define USERS_H

#include <stdint.h>
#include <stdlib.h>

#include "libargon2/argon2.h"

#define MAXUSERS 128
#define HT_PRIME1 151
#define HT_PRIME2 163
#define HASHLEN 32
#define SALTLEN 16
#define ENCODEDLEN 108
#define MAXRAWCRED 32


/////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////


typedef struct utemp {
    char* u;
    char* p;
} utemp;

utemp* parseLogin(char* toparse);

/* 
 * a single User
 * @param username: a char pointer storing username
 * @param salt: a uint8_t pointer storing the salt
 * @param hash: a uint8_t pointer storing the hash
*/
typedef struct User {
    char* username;
    uint8_t* salt;
    char* hash;
} User;

/*
 * a table of Users
 * @param capacity: maximum amount of Users
 * @param count: amount of Users currently populated
 * @param users: an array of User pointers
*/
typedef struct uTable {
    size_t capacity;
    size_t count;
    User** users;
} uTable;

/////////////////////////////////////////////////
// Functions - Manager
/////////////////////////////////////////////////

int uGenSalt(uint8_t* buffer, int bytes);
void encodedHashArgon(char* password, char* encoded, uint8_t* salt);
int loadStore(uTable* ut, const char* filepath);
void storeNewUser(uTable* ut, const char* filename, char* username, char* password);
int loginAsUser(uTable* ut, char* username, char* password);

/////////////////////////////////////////////////
// Functions - Storage
/////////////////////////////////////////////////

/*
 * calculate the hash
 * @param s: the value to find the hash of (username)
 * @param a: prime number
 * @param m: modulo
*/
int storageHash(const char* s, const int a, const int m);

/*
 * calculate the hash with attempts factored in (collision resistance)
 * @param s: the value to find the hash of (username)
 * @param users: modulo, amount of users
 * @param attempt: attempt counter
*/
int storageGetHash(const char* s, const int users, const int attempt);

/*
 * create the table storing users
 * generates MAXUSERS Users
*/
uTable* createuTable(void);

/*
 * create a User
 * @param username: username to be stored
 * @param salt: salt used for hashing (optional)
 * @param hash: storage for the hashed password
*/
User* createUser(char* username, uint8_t* salt, char* hash);

/*
 * create and insert a user into a table
 * @param ut: the table to insert into
 * @param username: username of the new user to be inserted
 * @param salt: salt used for hashing (optional)
 * @param hash: the hashed password value
*/
void insertUser(uTable* ut, char* username, uint8_t* salt, char* hash);

/*
 * iterate through a table to find a user via username
 * @param ut: the table to look through
 * @param username: the username to be searched for
*/
User* getUser(uTable* ut, const char* username);

/*
 * output the contents of a user table
 * @param ut: the table to be printed
*/
void dumpTable(uTable* ut);

// recursively free the users, then free the table
void destroyTable(uTable* ut);

// free a user
void destroyUser(User* u);


#endif // USERS_H
