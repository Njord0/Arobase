#ifndef _RANDOM_H
#define _RANDOM_H

#include <stdint.h>

int64_t random_intZintegerZinteger(int64_t min, int64_t max);
int _internal_get_random(int64_t *ptr);


#endif