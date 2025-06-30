/*
 *  locate.c
 *
 *  pull ips from server log and return some nice information about them.
 *
*/

#include <IP2Location.h>
#include <string.h>
#include <stdio.h>

#include "locate.h"

#define MAXIPS 128
#define IPLEN 16


//=====================================================================
// isDup() :: helper function to determine if an ip has been seen yet
//=====================================================================

int isDup(char* ip, char seen[][IPLEN], int count)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(ip, seen[i]) == 0) {
            return 1; // duplicate
        }
    }
    return 0; // not duplicate
}


//=====================================================================================
// pullAddressesFromLog() :: parse ips from server-ip.log and store each new ip address
//=====================================================================================

void pullAddressesFromLog(void)
{
    // source file :: to be parsed
    FILE* src = fopen("../logs/server-ip.log", "r");
    if (!src) {
        fprintf(stderr, "bad src\n");
        return;
    }

    // destination file :: to store parsed ips 
    FILE* dst = fopen("tests/locatelist.txt", "w");
    if (!dst) {
        fprintf(stderr, "bad dst\n");
        return;
    }

    char line[256];
    if (!fgets(line, sizeof(line), src)) {
        fprintf(stderr, "error reading src\n");
        return;
    }

    //===========================
    // Get each line, call sscanf() with ip formatting string.
    // Check if duplicate.
    // If not a duplicate, print it to destination file and
    //      copy it to the seen array.
    // Otherwise, skip it.
    //===========================
    char seen[MAXIPS][IPLEN];
    int seencnt = 0;
    int dupecnt = 0;
    while (fgets(line, sizeof(line), src) != 0) {
        char addr[16];
        if (sscanf(line, "%*[^0-9]%15[0-9.]", addr) == 1) {
            if (!isDup(addr, seen, seencnt)) {
                fprintf(dst, "%s", addr);
                fprintf(dst, "\n");
                strncpy(seen[seencnt++], addr, IPLEN);
            } else {
                dupecnt++;
            }
        }
    }
    printf("\nskipped [%d] duplicates\n", dupecnt);
    fclose(src);
    fclose(dst);
}


//=====================================================================
// printSingleAddressInfo() :: pass an ip and get its location data
//=====================================================================

int printSingleAddressInfo(char* ip)
{
    IP2Location* IP2LocationObj = IP2Location_open("IP2LOCATION-LITE-DB9.BIN");

    IP2LocationRecord* record = NULL;

    if (IP2LocationObj == NULL) {
        printf("db error\n");
        return -1;
    }

    if (IP2Location_open_mem(IP2LocationObj, IP2LOCATION_SHARED_MEMORY) == -1) {
        fprintf(stderr, "open_mem failed\n");
        return -1;
    }

    record = IP2Location_get_all(IP2LocationObj, ip);

    char latlong[50];

    sprintf(latlong, "%f, %f", record->latitude, record->longitude);
    printf("##               %-25s  ##\n", ip);
    printf("## | Country | %26s | ##\n", record->country_long);
    printf("## | Region  | %26s | ##\n", record->region);
    printf("## |  City   | %26s | ##\n", record->city);
    printf("## | Coords  | %26s | ##\n", latlong);
    printf("## |   Zip   | %26s | ##\n", record->zipcode);

    IP2Location_free_record(record);

    IP2Location_close(IP2LocationObj);

    IP2Location_delete_shm();
    return 0;
}


int printLoggedAddressesInfo(void)
{
    FILE* f;                // file to pull ips from
    char ipAddress[30];     // ip storage

    // generate fresh list from log 
    pullAddressesFromLog();

    IP2Location* IP2LocationObj = IP2Location_open("IP2LOCATION-LITE-DB9.BIN");
    if (IP2LocationObj == NULL) {
        printf("db error\n");
        return -1;
    }

    // NULL the record
    IP2LocationRecord* record = NULL;

    printf("--------VERSION--------\n");
    printf("IP2Loc api ver %s (%lu)\n", IP2Location_api_version_string(), IP2Location_api_version_num());
    // proc shit
    fprintf(stdout, "BIN ver: %s\n", IP2Location_bin_version(IP2LocationObj));
    printf("-----------------------\n\n");

    // memcheck
    if (IP2Location_open_mem(IP2LocationObj, IP2LOCATION_SHARED_MEMORY) == -1) {
        fprintf(stderr, "open_mem failed\n");
        return -1;
    }

    f = fopen("locatelist.txt", "r");

    // store location data in record and print
    while (fscanf(f, "%s", ipAddress) != EOF) {

        record = IP2Location_get_all(IP2LocationObj, ipAddress);

        if (record != NULL) {

            char latlong[50];

            sprintf(latlong, "%f, %f", record->latitude, record->longitude);
/*****/     printf("##############################################\n");
/* S */     printf("##               %-25s  ##\n", ipAddress);
/* O */     printf("##############################################\n");
/*   */     printf("## +--------------------------------------+ ##\n");
/* P */     printf("## | Country | %26s | ##\n", record->country_long);
/* R */     printf("## |---------|----------------------------| ##\n");
/* E */     printf("## | Region  | %26s | ##\n", record->region);
/* T */     printf("## |---------|----------------------------| ##\n");
/* T */     printf("## |  City   | %26s | ##\n", record->city);
/* T */     printf("## |---------|----------------------------| ##\n");
/* T */     printf("## | Coords  | %26s | ##\n", latlong);
/* T */     printf("## |---------|----------------------------| ##\n");
/* T */     printf("## |   Zip   | %26s | ##\n", record->zipcode);
/* Y */     printf("## +--------------------------------------+ ##\n");
/*****/     printf("##############################################\n\n");

        }

        // free record
        
        IP2Location_free_record(record);

    }

    // tidy up

    fclose(f);
    IP2Location_close(IP2LocationObj);
    IP2Location_delete_shm();

    return 0;
}
