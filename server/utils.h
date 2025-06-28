#ifndef UTILS_H
#define UTILS_H

//===================================
// functions
//===================================

/* decodes a string with url encoding */
void urldecode(char* dest, const char* source);

/* logs client IP */
void logIP(const char* format, ...);

#endif // UTILS_H
