#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>

#define TABLE_SIZE 256
#define INITBASESIZE 256
#define HT_PRIME1 151
#define HT_PRIME2 163

typedef struct Item {
    char* id;
    char* path;
    size_t size;
    struct Item* next;
} Item;

typedef struct Table {
    int base;
    int size;
    int count;
    Item** items;
} Table;

/* prototypes ***************************************************/

/* Takes in id string to be hashed */
int hash(const char* id);

/* Takes in id string, prime number to hash with, and table size */
int htHash(const char* s, const int a, const int m);

/* Takes in id string, items count, and attempt counter */
int htGetHash(const char* s, const int items, const int attempt);

/* Calls createTableSized() */
Table* createTable(void);

Table* createTableSized(const int base);

void htResize(Table* t, const int base);

void htResizeUp(Table* t);

void htResizeDown(Table* t);

/* Takes in id string, path, and size to place into new Item */
Item* createItem(const char* id, const char* path, size_t size);

/* Takes in table to insert into, id string, path, and filesize */
void insertItem(Table* t, const char* id, const char* path, size_t size);

/* Takes in the table to search through and the id string to search for */
Item* getItem(Table* t, const char* id);

/*  */
char* htSearch(Table* t, const char* key);

/* Takes the table and the id string to be deleted */
void htDelete(Table* t, const char* key);

/* Takes in the item to be deleted */
void htDeleteItem(Item* i);

void freeTable(Table* t);

void printTable(Table* t);

void loadClipsFromDir(Table* t, const char* directory);

void iterateClips(Table* t, void (*callback)(Item* item, void* ctx), void* ctx);

#endif
