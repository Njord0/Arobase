#include <stdint.h>
#include <string.h>

#include "file.h"

int64_t
file_openZstring(const char *name)
{
    int64_t out = -1;


    asm("mov $2, %%rax\n\t"
        "mov %1, %%rdi\n\t"
        "mov $2, %%rsi\n\t"
        "syscall\n\t"
        "mov %%rax, %0\n\t"
        : "=r"(out)
        : "c"(name)
        :);

    return out;
}

int64_t
file_readZintegerZbyteArr(int64_t fd, int64_t arr[])
{
    int64_t size = arr[0];
    uint8_t *ptr = (uint8_t*)arr+8;

    int64_t out = -1;

    asm("mov $0, %%rax\n\t"
        "syscall\n\t"
        "mov %%rax, %0"
        : "=r"(out)
        : "D"(fd), "S"(ptr), "d"(size)
        : "memory"
        );

    return out;
}

int64_t
file_readZintegerZcharArr(int64_t fd, int64_t arr[])
{
    int64_t size = arr[0];
    uint8_t *ptr = (uint8_t*)arr+8;

    int64_t out = -1;

    asm("mov $0, %%rax\n\t"
        "syscall\n\t"
        "mov %%rax, %0"
        : "=r"(out)
        : "D"(fd), "S"(ptr), "d"(size)
        : "memory"
        );

    return out;

}

void
file_writeZintegerZbyteArr(int64_t fd, int64_t arr[])
{
    int64_t size = arr[0];
    uint8_t *ptr = (uint8_t*)arr+8;

    asm("mov $1, %%rax\n\t"
        "syscall\n\n"
        :
        : "D"(fd), "S"(ptr), "d"(size)
        :
        );
}

void
file_writeZintegerZcharArr(int64_t fd, int64_t arr[])
{
    int64_t size = arr[0];
    uint8_t *ptr = (uint8_t*)arr+8;

    asm("mov $1, %%rax\n\t"
        "syscall\n\n"
        :
        : "D"(fd), "S"(ptr), "d"(size)
        :
        );
}

void
file_writeZintegerZstring(int64_t fd, const char *ptr)
{
    int64_t size = (int64_t)strlen(ptr);

    asm("mov $1, %%rax\n\t"
        "syscall\n\t"
        :
        : "D"(fd), "S"(ptr), "d"(size)
        :
        );
}

void
file_setposZintegerZinteger(int64_t fd, int64_t pos)
{
    asm("mov $8, %%rax\n\t"
        "mov $0, %%rdx\n\t"
        "syscall\n\t"
        :
        : "D"(fd), "S"(pos)
        :
        );
}

void
file_closeZinteger(int64_t fd)
{
    asm("mov $3, %%rax\n\t"
        "mov %0, %%rdi\n\t"
        "syscall\n\t"
        :
        : "r"(fd)
        :);
}