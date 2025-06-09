// hashtable.c

#include <stdio.h>
#include <time.h>
#include <stddef.h>

#define TABLE_SIZE 150

/* structures ***************************************************/

typedef struct {
    char* id;
    char* path;
    char* title;
    time_t timestamp;
    size_t filesize;
} VideoClip;
typedef struct {
    int base_size;
    int size;
    int count;
    VideoClip** clips;
} VideoTable;

/* hashing ******************************************************/

unsigned long
djb2(const char* s)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
}


