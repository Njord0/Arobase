#ifndef _ARRAY_H
#define _ARRAY_H


int64_t array_get_element(int64_t arr[], int64_t ind);

void array_set_element(int64_t arr[], int64_t ind, int64_t el);

int64_t _internal_bound_check(int64_t arr[], int64_t ind);


#endif