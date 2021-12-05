#include <stdint.h>

#include "array.h"

int64_t
array_get_element(int64_t arr[], int64_t ind)
{
    if (_internal_bound_check(arr, ind) == -1)
        return -1;

    return arr[ind+1];
}

uint8_t
array_get_element_c(int64_t arr[], int64_t ind)
{
    if (_internal_bound_check(arr, ind) == -1)
        return -1;

    uint8_t *ptr = (uint8_t*)arr+8;

    return ptr[ind];
}

double
array_get_element_f(double arr[], int64_t ind)
{
    int size = *(int*)arr;
    if ((ind >= size) || (ind < 0))
        return -1.0;

    return arr[ind+1];
}

void
array_set_element(int64_t arr[], int64_t ind, int64_t el)
{
    if (_internal_bound_check(arr, ind) == -1)
        return;

    arr[ind+1] = el;
}

void
array_set_element_c(int64_t arr[], int64_t ind, int64_t el)
{
    if (_internal_bound_check(arr, ind) == -1)
        return;

    uint8_t *ptr = (uint8_t*)arr+8;

    ptr[ind] = (uint8_t)el;
}

void
array_set_element_f(double arr[], int64_t ind, double el)
{
    int size = *(int*)arr;
    if ((ind >= size) || (ind < 0))
        return;

    arr[ind+1] = el;
}

int64_t
_internal_bound_check(int64_t arr[], int64_t ind)
{
    int64_t size = arr[0];

    if ((ind >= size) || (ind < 0))
        return -1;

    return 1;
}

int64_t
_internal_get_sizeZintegerArr(int64_t arr[])
{
    return arr[0];
}

int64_t
_internal_get_sizeZcharArr(int64_t arr[])
{
    return arr[0];
}

int64_t
_internal_get_sizeZbyteArr(int64_t arr[])
{
    return arr[0];
}

int64_t
_internal_get_sizeZfloatArr(double arr[])
{
    return *(int64_t*)arr;
}