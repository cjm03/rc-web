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
    Item** items;
    int base;
    int size;
    int count;
} Table;

//==================================================================================================
// hash functions
//==================================================================================================

/* Takes in id string to be hashed */
int hash(const char* id);

/* Takes in id string, prime number to hash with, and table size */
int htHash(const char* s, const int a, const int m);

/* Takes in id string, items count, and attempt counter */
int htGetHash(const char* s, const int items, const int attempt);


//==================================================================================================
// creation/allocation functions
//==================================================================================================

/* Calls createTableSized with INITBASESIZE */
Table* createTable(void);

/* returns default initialized hashtable with provided size */
Table* createTableSized(const int base);


//==================================================================================================
// resizing functions
//==================================================================================================

/* creates a temp table, stores current data in it, resizes original table and returns the data */
void htResize(Table* t, const int base);

/* makes the table larger */
void htResizeUp(Table* t);

/* makes the table smaller */
void htResizeDown(Table* t);


//==================================================================================================
// insertion/navigation functions
//==================================================================================================

/* Takes in id string, path, and size to place into new Item */
Item* createItem(const char* id, const char* path, size_t size);

/* Takes in table to insert into, id string, path, and filesize */
void insertItem(Table* t, const char* id, const char* path, size_t size);

/* Takes in the table to search through and the id string to search for. Returns the whole item */
Item* getItem(Table* t, const char* id);

/* Takes in a table and id string, however only returns the value associated with the id */
char* htSearch(Table* t, const char* id);

//==================================================================================================
// removal functions
//==================================================================================================

/* Takes the table and the id string to be deleted */
void htDelete(Table* t, const char* key);

/* Takes in the item to be deleted */
void htDeleteItem(Item* i);

/* Deallocates an entire table and all of its items */
void freeTable(Table* t);

//==================================================================================================
// output hashtable contents
//==================================================================================================

void printTable(Table* t);

//==================================================================================================
// populate a hashtable 
//==================================================================================================

void loadClipsFromDir(Table* t, const char* directory);

//==================================================================================================
// helper for making .json
//==================================================================================================

void iterateClips(Table* t, void (*callback)(Item* item, void* ctx), void* ctx);

#endif
