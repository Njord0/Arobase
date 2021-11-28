#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "print.h"
#include "type/string.h"

void
print_integer(int64_t integer)
{

    if (integer == 0)
    {
        _internal_print("0");
        return;
    }

    char out[23];

    int64_t tmp = integer;

    if (tmp == -1)
    {
        tmp--;
        tmp *= -1;
    }
    

    else if (integer < 0)
        tmp *= -1;

    unsigned int pos = 0;

    while (tmp != 0)
    {
        out[pos] = 48 + (tmp % 10);
        tmp = tmp/10;
        pos++;
    }

    out[pos] = '\x00';
    
    _internal_string_reverse(out);

    if (integer < 0)
        _internal_print("-");

    _internal_print(out);
}

void
print_char(char c)
{
    char array[2] = {c, '\x00'};
    _internal_print(array);
}

void
print_string(const char *ptr)
{
    _internal_print(ptr);
}

void
_internal_print(const char *ptr)
{
    size_t len = _internal_string_len(ptr);
    asm("movq $1, %%rax\n\t"
        "movq $0, %%rdi\n\t"
        "mov %0, %%rsi\n\t"
        "mov %1, %%edx\n\t"
        "syscall\n\t"
        :
        : "c"(ptr), "r"((unsigned int)len):);

}

void
_internal_assert(const char *ptr)
{
    _internal_print("Assertion failed : \n\t");
    if (ptr != NULL)
        _internal_print(ptr);
    asm("movq $60, %%rax\n\t"
        "movq $1, %%rdi\n\t"
        "syscall\n\t"
        :::);
}