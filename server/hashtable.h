#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <time.h>
#include <stdbool.h>
#include <stddef.h>

#define TABLE_SIZE 10

/* structures ***************************************************/

typedef struct {
    char* key;
    char* value;
} htClip;

// typedef struct {
//     int size;
//     int count;
//     htClip** clips;
// } htTable;

/* prototypes ***************************************************/

unsigned long djb2(const char* s);
void initHashTable(void);
void displayHashTable(void);
htClip* htNewClip(const char* id, const char* path);
bool htInsert(htClip* c);
htClip* htSearch(const char* id);
htClip* htDelete(char* id);

// static htClip* htNewClip(const char* id, const char* path);
// htTable* htNewTable(void);
// void htInsertClip(htTable* ht, const char* id, const char* path);
// char* htSearch(htTable* ht, const char* id);
// void htDelTable(htTable* ht);

#endif
