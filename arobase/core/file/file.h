#ifndef _AROBASE_FILE_H
#define _AROBASE_FILE_H

#include <stdint.h>

int64_t file_openZstring(const char *name);

int64_t file_readZintegerZbyteArr(int64_t fd, int64_t arr[]);
int64_t file_readZintegerZcharArr(int64_t fd, int64_t arr[]);

void file_writeZintegerZbyteArr(int64_t fd, int64_t arr[]);
void file_writeZintegerZcharArr(int64_t fd, int64_t arr[]);
void file_writeZintegerZstring(int64_t fd, const char *ptr);

void file_setposZintegerZinteger(int64_t fd, int64_t pos);

void file_closeZinteger(int64_t fd);
    

#endif