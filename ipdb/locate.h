#ifndef LOCATE_H
#define LOCATE_H

#define MAXIPS 128
#define IPLEN 16

/* function to avoid duplicate ips */
int isDup(char* ip, char seen[][IPLEN], int count);

/* parse ips from log and store , assuming no duplicate */
void pullAddressesFromLog(void);

/* prints ip info about passed ip. callable externally using locate.h */
int printSingleAddressInfo(char* ip);

/* uses pullAddressesFromLog to populate list and then print the info */
int printLoggedAddressesInfo(void);

#endif // LOCATE_H
