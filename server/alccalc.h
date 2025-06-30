#ifndef ALCCALC_H
#define ALCCALC_H

#define ITEMS 6
#define DISCOUNT 0.15

typedef struct Discount {
    float* orig;
    float* disc;
    float* newp;
    float totaldisc;
    float totalcost;
} Discount;

void parseInput(Discount* t, char* data);

Discount* create(void);

void calcDisc(Discount* t);

void freeDisc(Discount* t);

#endif // ALCCALC_H
