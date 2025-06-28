#ifndef LOCATE_H
#define LOCATE_H

#define MAXIPS 128
#define IPLEN 16

int isDup(char* ip, char seen[][IPLEN], int count);

void pullAddressesFromLog(void);

int printSingleAddressInfo(char* ip);

#endif
