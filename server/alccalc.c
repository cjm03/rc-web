#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alccalc.h"


void parseInput(float prices[], char* data)
{
    char* token = strtok(data, "&");
    int i = 0;

    while (token != NULL && i < ITEMS) {
        char* eq = strchr(token, '=');
        if (eq) {
            prices[i] = atof(eq + 1);
            i++;
        }
        token = strtok(NULL, "&");
    }
}

char calcDisc(const char* posted)
{
    char respbuf[4096];
    float bottles[ITEMS], discount[ITEMS], newbottles[ITEMS];
    float totaldisc, totalcost;

    char* data = strdup(posted);
    char* token = strtok(data, "&");
    int i = 0;
    while (token && i < ITEMS) {
        char* eq = strchr(token, '=');
        if (eq) {
            bottles[i] = atof(eq + 1);
        }
        token = strtok(NULL, "&");
        i++;
    }


    snprintf(respbuf, sizeof(respbuf) - 1,
        "<!DOCTYPE html>"
        "<html lang='en'>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<title>Discounts</title>"
        "</head>"
        "<body>"
        "<h2>Disc Calc</h2>"
        "<table border='1'>"
        "<tr>"
        "<th>Item</th>"
        "<th>Original</th>"
        "<th>Discount</th>"
        "<th>New Price</th>"
        "</tr>");


    for (int i = 0; i < ITEMS; i++) {
        discount[i] = bottles[i] * DISCOUNT;
        newbottles[i] = bottles[i] - discount[i];
        totaldisc += discount[i];
        totalcost += newbottles[i];

        char row[256];
        snprintf(row, sizeof(row),
            "<tr><td>%d</td><td>$%.2f</td><td>$%.2f</td><td>$%.2f</td></tr>",
            i + 1, bottles[i], discount[i], newbottles[i]);
        strcat(respbuf, row);
    }
    char summary[256];
    snprintf(summary, sizeof(summary),
        "</table><h3>Total Discount: $%.2f</h3>"
        "<h3>Total After Discount: $%.2f</h3></body></html>",
        totaldisc, totalcost);
    strcat(respbuf, summary);

    free(data);
    return *respbuf;
}
