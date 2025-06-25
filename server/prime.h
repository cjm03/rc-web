#ifndef PRIME_H
#define PRIME_H

//=====================================
// functions
//=====================================

/* takes in an integer and determines primeness */
/* Returns 1 if prime, 0 if not prime, and -1 if the integer is 0, 1, 2, or negative */
int isPrime(const int x);

/* determines the next prime number following the integer passed into it */
int nextPrime(int x);

#endif // PRIME_H
