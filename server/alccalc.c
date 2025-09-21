#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "alccalc.h"


void parseDiscountInput(Discount* t, char* data)
{
    size_t dlen = strlen(data);
    char copy[dlen];
    printf("DATA: [%s]\n", data);
    printf("copy: [%s]\n", copy);

    printf("0-4: %c %c %c %c %c\n", data[0], data[1], data[2], data[3], data[4]);
    strncpy(copy, data, dlen);
    char* token = strtok(copy, "&");
    int i = 0;

    while (token != NULL && i < ITEMS) {
        char* eq = strchr(token, '=');
        if (eq) {
            t->orig[i] = atof(eq + 1);
            i++;
        }
        token = strtok(NULL, "&");
    }
    free(token);
    return;
}

Discount* createDiscountTable(void)
{
    Discount* t = malloc(sizeof(Discount));
    t->orig = calloc(ITEMS, sizeof(float*) * 2);
    t->disc = calloc(ITEMS, sizeof(float*) * 2);
    t->newp = calloc(ITEMS, sizeof(float*) * 2);
    t->totalorig = 0.0;
    t->totaldisc = 0.0;
    t->totalcost = 0.0;
    return t;
}

void calculateDiscount(Discount* t)
{
    for (int j = 0; j < ITEMS; j++) {
        t->disc[j] = t->orig[j] * DISCOUNT;
        t->newp[j] = t->orig[j] - t->disc[j];
        t->totalorig += t->orig[j];
        t->totaldisc += t->disc[j];
        t->totalcost += t->newp[j];
    }
    return;
}

void freeDiscountTable(Discount* t)
{
    for (int k = 0; k < ITEMS; k++) {
        t->orig[k] = 0.0;
        t->disc[k] = 0.0;
        t->newp[k] = 0.0;
    }
    free(t);
}
