#include <stdio.h>

#include "flate.h"

int main(void)
{
    Flate* f = NULL;

    flateSetFile(&f, "discounted.html");

    printf("\n");

    return 0;

}
