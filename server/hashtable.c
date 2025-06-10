/*
 *  hashtable.c
 *
 *  Store video metadata for fast lookup
*/

#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "hashtable.h"


static htClip* htTable[TABLE_SIZE];

/* hash *********************************************************/

unsigned long djb2(const char* s)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
}

/* functions ****************************************************/

void initHashTable(void)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        htTable[i] = NULL;
    }
}

void displayHashTable(void)
{
    printf("Start\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (htTable[i] == NULL) {
            printf("\tK: -------- P: --------\n");
        } else {
            printf("\tK: %s P: --------\n", htTable[i]->key);
        }
    }
    printf("End\n");
}

htClip* htNewClip(const char* id, const char* path)
{
    // unsigned long index = djb2(id);
    htClip* newClip = malloc(sizeof(htClip));
    newClip->key = strdup(id);
    newClip->value = strdup(path);
    return newClip;
}

bool htInsert(htClip *c)
{
    if (c == NULL) return false;
    int index = djb2(c->key);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if (htTable[try] == NULL) {
            htTable[try] = c;
            return true;
        }
    }
    return false;
}

htClip* htSearch(const char* key)
{
    int index = djb2(key);
    if (htTable[index] != NULL && strncmp(htTable[index]->key, key, TABLE_SIZE) == 0) {
        return htTable[index];
    } else {
        return NULL;
    }
}

htClip* htDelete(char* key)
{
    int index = djb2(key);
    if (htTable[index] != NULL && strncmp(htTable[index]->key, key, TABLE_SIZE) == 0) {
        // htClip* tmp = htTable[index];
        htTable[index] = NULL;
        return htTable[index];
    } else {
        return NULL;
    }
}

/* main ********************************************************************/







