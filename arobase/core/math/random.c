#include <stdlib.h>
#include <stdint.h>

#include "random.h"

int64_t random_intZintegerZinteger(int64_t min, int64_t max)
{
    int64_t val;
    if (_internal_get_random(&val) == -1)
        return (int64_t)-1;

    if (val < 0)
        val *= -1;

    return val % (max - min + 1) + min;
}

int _internal_get_random(int64_t *ptr)
{
    ssize_t ret = 0;

    asm("movq $318, %%rax\n\t" // sys_get_random
    "movq %1, %%rdi\n\t"
    "mov $8, %%rsi\n\t"
    "mov $1, %%edx\n\t"
    "syscall\n\t"
    "mov %%rax, %0\n\t"
    : "=r"(ret)
    : "c"(ptr):);

    return (int)ret;
}
