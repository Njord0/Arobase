#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "input.h"

int64_t
input_integer()
{
    int64_t i = 0;

    char buf[23];

    if (_internal_read(buf, 21) == 21)
        _internal_flush();
   
    /* Converting to integer */

    bool is_negative = (buf[0] == '-');

    char *c = buf;
    if (is_negative)
        c++;

    while ((*c != '\x00') && (*c >= '0') && (*c <= '9'))
    {
        i = (*c - '0') + i*10;
        c++;
    }
    
    if (is_negative)
        i *= -1;

    return i; 
}

double
input_float()
{
    double value;
    scanf("%lf", &value);
    _internal_flush();
    return value;
}

char
input_char()
{
    char buf[2] = {0};
    _internal_read(buf, 1);

    _internal_flush();

    if (buf[0] >= '0' && buf[0] <= '9')
        return buf[0] - 0x30;

    return buf[0];
}

char
input_byte()
{
    char buf[2] = {0};
    _internal_read(buf, 1);

    _internal_flush();

    return buf[0];
}


ssize_t
_internal_read(char *ptr, unsigned int len)
{

    ssize_t size = 0;

    asm("movq $0, %%rax\n\t"
    "movq $1, %%rdi\n\t"
    "mov %1, %%rsi\n\t"
    "mov %2, %%edx\n\t"
    "syscall\n\t"
    "mov %0, %%rax\n\t"
    : "=r"(size)
    : "c"(ptr), "r"((unsigned int)len):);

    return size;

}

void
_internal_flush()
{
    char c[2] = {0};

    while (*c != '\n' && *c != '\0')
        _internal_read(c, 1);
    
    
}