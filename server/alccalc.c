#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "alccalc.h"


void parseInput(Discount* t, char* data)
{
    size_t dlen = strlen(data);
    char copy[dlen];
    strncpy(copy, data, dlen);
    char* token = strtok(copy, "&");
    int i = 0;

    while (token != NULL && i < ITEMS) {
        char* eq = strchr(token, '=');
        if (eq) {
            t->orig[i] = atof(eq + 1);
            printf("%.2f\n", t->orig[i]);
            i++;
        }
        token = strtok(NULL, "&");
    }
    free(token);
    // for (int x = 0; x < 6; x++) printf("%.2f\n", t->orig[i]);
    return;
}

Discount* create(void)
{
    Discount* t = malloc(sizeof(Discount));
    t->orig = calloc(ITEMS, sizeof(float*) * 2);
    t->disc = calloc(ITEMS, sizeof(float*) * 2);
    t->newp = calloc(ITEMS, sizeof(float*) * 2);
    t->totaldisc = 0.0;
    t->totalcost = 0.0;
    return t;
}

void calcDisc(Discount* t)
{
    for (int j = 0; j < ITEMS; j++) {
        t->disc[j] = t->orig[j] * DISCOUNT;
        t->newp[j] = t->orig[j] - t->disc[j];
        t->totaldisc += t->disc[j];
        t->totalcost += t->newp[j];
    }
    return;
}

void freeDisc(Discount* t)
{
    free(t->orig);
    free(t->disc);
    free(t->newp);
    free(t);
}
