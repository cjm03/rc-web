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


static Clip* Table[TABLE_SIZE];

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

void initTable(void)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        Table[i] = NULL;
    }
}

void insertClip(const char* id, const char* filename, size_t filesize)
{
    unsigned long index = djb2(id);
    Clip* newclip = malloc(sizeof(Clip));
    newclip->id = strdup(id);
    newclip->filename = strdup(filename);
    newclip->filesize = filesize;
    newclip->next = Table[index];
    Table[index] = newclip;
}

Clip* getClip(const char* id)
{
    unsigned long index = djb2(id);
    Clip* cur = Table[index];
    while (cur) {
        if (strcmp(cur->id, id) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

void freeTable(void)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        Clip* cur = Table[i];
        while (cur) {
            Clip* next = cur->next;
            free(cur->id);
            free(cur->filename);
            free(cur);
            cur = next;
        }
    }
}

void iterateClips(void (*callback)(Clip *clip, void *ctx), void *ctx)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        Clip* cur = Table[i];
        while (cur) {
            callback(cur, ctx);
            cur = cur->next;
        }
    }
}

// void displayHashTable(void)
// {
//     printf("Start\n");
//     for (int i = 0; i < TABLE_SIZE; i++) {
//         if (htTable[i] == NULL) {
//             printf("\tK: -------- P: --------\n");
//         } else {
//             printf("\tK: %s P: --------\n", htTable[i]->key);
//         }
//     }
//     printf("End\n");
// }

/* main ********************************************************************/







