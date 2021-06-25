#include <stdlib.h>
#include <stdint.h>

#include "cast.h"

int64_t char_to_intZchar(char c)
{
    return (int64_t)c;
}

char int_to_charZinteger(int64_t i)
{
    return (char)i;
}

char byte_to_charZbyte(char c)
{
    return c;
}

char char_to_byteZchar(char c)
{
    return c;
}