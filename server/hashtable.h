#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <time.h>
#include <stdbool.h>
#include <stddef.h>

#include "clip.h"

#define TABLE_SIZE 10

/* prototypes ***************************************************/

void initTable(void);
void insertClip(const char* id, const char* filename, size_t filesize);
Clip* getClip(const char* id);
void freeTable(void);

void iterateClips(void (*callback)(Clip* clip, void *ctx), void *ctx);

// unsigned long djb2(const char* s);
// void initHashTable(void);
// void displayHashTable(void);
// htClip* htNewClip(const char* id, const char* path);
// bool htInsert(htClip* c);
// htClip* htSearch(const char* id);
// htClip* htDelete(char* id);

#endif
