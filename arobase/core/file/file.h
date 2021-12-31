#ifndef _AROBASE_FILE_H
#define _AROBASE_FILE_H

#include <stdint.h>

int64_t fopenZstring(const char *name);

int64_t freadZintegerZbyteArr(int64_t fd, int64_t arr[]);
int64_t freadZintegerZcharArr(int64_t fd, int64_t arr[]);
int64_t freadZintegerZintegerArr(int64_t fd, int64_t arr[]);
int64_t freadZintegerZfloatArr(int64_t fd, int64_t arr[]);

void fwriteZintegerZbyteArr(int64_t fd, int64_t arr[]);
void fwriteZintegerZcharArr(int64_t fd, int64_t arr[]);
void fwriteZintegerZintegerArr(int64_t fd, int64_t arr[]);
void fwriteZintegerZfloatArr(int64_t fd, int64_t arr[]);

void fwriteZintegerZinteger(int64_t fd, int64_t i);
void fwriteZintegerZbyte(int64_t fd, char c);
void fwriteZintegerZchar(int64_t fd, char c);
void fwriteZintegerZfloat(int64_t fd, double f);
void fwriteZintegerZstring(int64_t fd, const char *ptr);

void ftouchZstring(const char *name);

void fcloseZinteger(int64_t fd);
    

#endif