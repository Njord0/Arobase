#include <stdlib.h>
#include <stdint.h>

#include "cast.h"

char
to_byteZinteger(int64_t i)
{
    return (char)i;
}

int64_t
to_intZbyte(char c)
{
    return (int64_t)c;
}

int64_t
to_intZchar(char c)
{
    return (int64_t)c;
}

char
to_charZinteger(int64_t i)
{
    return (char)i;
}

char
to_charZbyte(char c)
{
    return c;
}

char
to_byteZchar(char c)
{
    return c;
}

double
to_floatZinteger(int64_t i)
{
    return (double)i;
}

int64_t
to_intZfloat(double d)
{
    return (int)d;
}