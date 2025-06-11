#ifndef CLIP_H
#define CLIP_H

#include <stddef.h>

typedef struct Clip {
    char* id;
    char* filename;
    size_t filesize;
    struct Clip* next;
} Clip;

#endif
