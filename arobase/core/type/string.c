#include <stddef.h>

#include "string.h"

size_t
_internal_string_len(const char *ptr)
{
    size_t t = 0;

    while (*ptr != '\x00')
    {
        t++; 
        ptr++;
    }

    return t;
}

void
_internal_string_reverse(char *ptr)
{
    char tmp;
    size_t len = _internal_string_len(ptr);
    for (unsigned int i = 0; i < len/2; i++)
    {
        tmp = ptr[i];
        ptr[i] = ptr[len-i-1];
        ptr[len-i-1] = tmp;
    }
}