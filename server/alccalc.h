#ifndef ALCCALC_H
#define ALCCALC_H

#define ITEMS 6
#define DISCOUNT 0.15

typedef struct Discount {
    float* orig;
    float* disc;
    float* newp;
    float totalorig;
    float totaldisc;
    float totalcost;
} Discount;

void parseDiscountInput(Discount* t, char* data);

Discount* createDiscountTable(void);

void calculateDiscount(Discount* t);

void freeDiscountTable(Discount* t);

#endif // ALCCALC_H
