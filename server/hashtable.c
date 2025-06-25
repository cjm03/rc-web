//===========================================================================================================================================
//  hashtable.c
//  Store video metadata for fast lookup
//===========================================================================================================================================

#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>

#include "hashtable.h"
#include "prime.h"

//===========================================================================================================================================
// deleted hashtable items
//===========================================================================================================================================

static Item DELETEDITEM = {NULL, NULL, 0, NULL};

//===========================================================================================================================================
// hash functions
//===========================================================================================================================================

int hash(const char* id)
{
    long hash = 0;
    int len = strlen(id);
    for (int i = 0; i < len; i++) {
        hash += (long)pow(151, len - (i + 1)) * id[i];
        hash = hash % TABLE_SIZE;
    }
    return (int)hash;
}

int htHash(const char* s, const int a, const int m)
{
    long hash = 0;
    const int len = strlen(s);
    for (int i = 0; i < len; i++) {
        hash += (long)pow(a, len - (i + 1)) * s[i];
        hash = hash % m;
    }
    return (int)hash;
}

int htGetHash(const char* s, const int items, const int attempt)
{
    const int hasha = htHash(s, HT_PRIME1, items);
    const int hashb = htHash(s, HT_PRIME2, items);
    return (hasha + (attempt * (hashb + 1))) % items;
}

//===========================================================================================================================================
// creation/allocation functions
//===========================================================================================================================================

Table* createTable(void)
{
    return createTableSized(INITBASESIZE);
}

Table* createTableSized(const int base)
{
    Table* t = malloc(sizeof(Table));
    t->base = base;
    t->size = nextPrime(t->base);
    t->count = 0;
    t->items = calloc((size_t)t->size, sizeof(Item*));
    // memset(t->items, 0, sizeof(t->items));
    return t;
}

Item* createItem(const char* id, const char* path, size_t size)
{
    Item* i = malloc(sizeof(Item));
    i->id = strdup(id);
    i->path = strdup(path);
    i->size = size;
    return i;
}

//===========================================================================================================================================
// resizing functions
//===========================================================================================================================================

void htResize(Table* t, const int base)
{
    if (base < INITBASESIZE) return;
    Table* new = createTableSized(base);
    for (int i = 0; i < t->size; i++) {
        Item* item = t->items[i];
        if (item != NULL && item != &DELETEDITEM) {
            insertItem(new, item->id, item->path, item->size);
        }
    }
    t->base = new->base;
    t->count = new->count;

    const int tempsize = t->size;
    t->size = new->size;
    new->size = tempsize;

    Item** tempitems = t->items;
    t->items = new->items;
    new->items = tempitems;

    freeTable(new);
}

void htResizeUp(Table* t)
{
    const int newsize = t->base * 2;
    htResize(t, newsize);
}

void htResizeDown(Table* t)
{
    const int newsize = t->base / 2;
    htResize(t, newsize);
}

//===========================================================================================================================================
// insertion/navigation functions
//===========================================================================================================================================

void insertItem(Table* t, const char* id, const char* path, size_t size)
{
    const int load = t->count * 100 / t->size;
    if (t->count != 0) {
        if (load > 70) {
            htResizeUp(t);
        }
    }
    Item* new = createItem(id, path, size);
    int index = htGetHash(new->id, t->size, 0);;
    Item* current = t->items[index];
    int i = 1;
    while (current != NULL) { 
        if (current != &DELETEDITEM) {
            if (strcmp(current->id, id) == 0) {
                htDeleteItem(current);
                t->items[index] = new;
                return;
            }
        }
        index = htGetHash(new->id, t->size, i);
        current = t->items[index];
        i++;
    }
    t->items[index] = new;
    t->count++;
}

Item* getItem(Table* t, const char* id)
{
    int index = htGetHash(id, t->size, 0);
    Item* cur = t->items[index];
    int i = 1;
    while (cur != NULL) {
        if (cur != &DELETEDITEM) {
            if (strcmp(cur->id, id) == 0) {
                return cur;
            }
        }
        index = htGetHash(id, t->size, i);
        cur = t->items[index];
        i++;
    }
    return NULL;
}

char* htSearch(Table* t, const char* id)
{
    int index = htGetHash(id, t->size, 0);
    Item* item = t->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &DELETEDITEM) {
            if (strcmp(item->id, id) == 0) {
                return item->path;
            }
        }
        index = htGetHash(id, t->size, i);
        item = t->items[index];
        i++;
    }
    return NULL;
}

//===========================================================================================================================================
// removal functions
//===========================================================================================================================================

void htDelete(Table* t, const char* id)
{
    const int load = t->count * 100 / t->size;
    if (load < 10) {
        htResizeDown(t);
    }
    int index = htGetHash(id, t->size, 0);
    Item* item = t->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &DELETEDITEM) {
            if (strcmp(item->id, id) == 0) {
                htDeleteItem(item);
                t->items[index] = &DELETEDITEM;
            }
        }
        index = htGetHash(id, t->size, i);
        item = t->items[index];
        i++;
    }
    t->count--;
}

void htDeleteItem(Item* i)
{
    free(i->id);
    free(i->path);
    free(i);
}

void freeTable(Table* t)
{
    for (int i = 0; i < t->size; ++i) {
        Item* cur = t->items[i];
        if (cur != NULL) {
            htDeleteItem(cur);
        }
    }
    free(t->items);
    free(t);
}

//===========================================================================================================================================
// printTable - Shittily output the contents of the hash table
//===========================================================================================================================================

void printTable(Table* t)
{
    printf("|--------------------------------------------------|------------------------------------------------------------------|------------------|\n");
    printf("| %48s | %64s | %16s |\n", "ID", "PATH", "SIZE");
    printf("|--------------------------------------------------|------------------------------------------------------------------|------------------|\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        Item* current = t->items[i];
        if (current != NULL) {
            printf("| %48s | %64s | %16ld |\n", current->id, current->path, current->size);
            printf("|--------------------------------------------------|------------------------------------------------------------------|------------------|\n");
        } /*else {
            printf("|--------------------------------------------------|------------------------------------------------------------------| %16d |\n", 0);
        } */
    }
    printf("\ncounts:\n\tt->base: %d\n\tt->size: %d\n\tt->count: %d\n", t->base, t->size, t->count);
}

//=================================
// loadClipsFromDir - Helper func
//=================================

void loadClipsFromDir(Table* t, const char* directory)
{
    DIR* dir = opendir(directory);
    if (!dir) {
        perror("opendir");
        return;
    }
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) {
            if (strstr(entry->d_name, ".mp4")) {
                char id[256] = {0};
                char filepath[512] = {0};

                snprintf(id, sizeof(id), "%.*s", (int)(strlen(entry->d_name) - 4), entry->d_name);
                
                snprintf(filepath, sizeof(filepath), "%s/%s", directory, entry->d_name);
                
                struct stat st;
                if (stat(filepath, &st) == 0) {
                    insertItem(t, id, filepath, st.st_size);
                    printf("Loaded clip: %s\n", id);
                }
            }
        }
    }
    closedir(dir);
}

//===================================
// iterateClips - Helper func
//===================================

void iterateClips(Table* t, void (*callback)(Item* item, void* ctx), void* ctx)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        Item* cur = t->items[i];
        while (cur != NULL) {
            callback(cur, ctx);
            cur = cur->next;
        }
    }
}






