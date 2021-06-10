#include <stdlib.h>
#include <stdint.h>

#include "cast.h"

int64_t char_to_int(char c)
{
    return (int64_t)c;
}

char int_to_char(int64_t i)
{
    return (char)i;
}

char byte_to_char(char c)
{
    return c;
}

char char_to_byte(char c)
{
    return c;
}