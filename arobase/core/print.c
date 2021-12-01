#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "print.h"

void
print_integer(int64_t integer)
{

    printf("%ld", integer);
}

void
print_char(char c)
{
    printf("%c", c);
}

void
print_string(const char *ptr)
{
    printf("%s", ptr);
}

void
_internal_assert(const char *ptr)
{
    printf("Assertion failed : \n\t");
    if (ptr)
        printf("%s", ptr);
    asm("movq $60, %%rax\n\t"
        "movq $1, %%rdi\n\t"
        "syscall\n\t"
        :::);
}