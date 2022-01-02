#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <fcntl.h>
#include "file.h"

FILE* opened[65535] = {NULL, };

int64_t
fopenZstring(const char *name)
{
    int64_t out = -1;

    FILE *f = fopen(name, "r+");
    if (f == NULL)
        return -1;

    out = fileno(f);
    opened[out] = f;

    fseek(f, 0 , SEEK_SET);

    return out;
}

int64_t
freadZintegerZbyteArr(int64_t fd, int64_t arr[])
{
    if (opened[fd] == NULL)
        return -1;

    int64_t size = arr[0];
    uint8_t *ptr = (uint8_t*)arr+8;

    for (int i = 0; i < size; i++)
        ptr[i] = (uint8_t)getc(opened[fd]);
    
    return 0;
}

int64_t
freadZintegerZcharArr(int64_t fd, int64_t arr[])
{
    if (opened[fd] == NULL)
        return -1;

    int64_t size = arr[0];
    uint8_t *ptr = (uint8_t*)arr+8;

    for (int i = 0; i < size; i++)
        ptr[i] = (uint8_t)getc(opened[fd]);
    
    return 0;
}

int64_t
freadZintegerZintegerArr(int64_t fd, int64_t arr[])
{
    if (opened[fd] == NULL)
        return -1;
    
    int64_t size = arr[0];
    int64_t *ptr = (int64_t*)arr+8;

    for (int i = 0; i < size; i++)
    {
        if (fscanf(opened[fd], "%ld", &ptr[i]) == EOF)
            return -1;
    }
    
    return 0;
}

int64_t
freadZintegerZfloatArr(int64_t fd, int64_t arr[])
{
    if (opened[fd] == NULL)
        return -1;
    
    int64_t size = arr[0];
    double *ptr = (double*)arr+8;

    for (int i = 0; i < size; i++)
    {
        if (fscanf(opened[fd], "%lf", &ptr[i]) == EOF)
            return -1;
    }

    return 0;
}

void
fwriteZintegerZbyteArr(int64_t fd, int64_t arr[])
{
    if (opened[fd] == NULL)
        return;

    int64_t size = arr[0];

    uint8_t *ptr = (uint8_t*)arr+8;

    for (int i = 0; i < size; i++)
        fprintf(opened[fd], "%d", ptr[i]);
    
}

void
fwriteZintegerZcharArr(int64_t fd, int64_t arr[])
{
    if (opened[fd] == NULL)
        return;

    int64_t size = arr[0];

    uint8_t *ptr = (uint8_t*)arr+8;

    for (int i = 0; i < size; i++)
        fprintf(opened[fd], "%c", ptr[i]);
    
}

void
fwriteZintegerZintegerArr(int64_t fd, int64_t arr[])
{
    if (opened[fd] == NULL)
        return;

    int64_t size = arr[0];

    uint64_t *ptr = (uint64_t*)arr+8;

    for (int i = 0; i < size; i++)
        fprintf(opened[fd], "%ld", ptr[i]);
}

void
fwriteZintegerZfloatArr(int64_t fd, int64_t arr[])
{
    if (opened[fd] == NULL)
        return;

    int64_t size = arr[0];

    double *ptr = (double*)arr+8;

    for (int i = 0; i < size; i++)
        fprintf(opened[fd], "%lf", ptr[i]);
}

void
fwriteZintegerZinteger(int64_t fd, int64_t i)
{
    if (opened[fd] == NULL)
        return;
    
    fprintf(opened[fd], "%ld", i);
}

void fwriteZintegerZbyte(int64_t fd, char c)
{
    if (opened[fd] == NULL)
        return;

    fprintf(opened[fd], "%d", c);
}

void
fwriteZintegerZchar(int64_t fd, char c)
{
    if (opened[fd] == NULL)
        return;
    
    fprintf(opened[fd], "%c", c);
}

void
fwriteZintegerZfloat(int64_t fd, double f)
{
    if (opened[fd] == NULL)
        return;
    
    fprintf(opened[fd], "%lf", f);
}


void
fwriteZintegerZstring(int64_t fd, const char *ptr)
{
    if (opened[fd] == NULL)
        return;

    fprintf(opened[fd], "%s", ptr);
}

void
ftouchZstring(const char *name)
{
    FILE *f = fopen(name, "w");
    if (f)
        fclose(f);   
}


void
fcloseZinteger(int64_t fd)
{
    if (opened[fd] == NULL)
        return;
    fclose(opened[fd]);
    opened[fd] = NULL;
}