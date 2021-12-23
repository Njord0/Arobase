#ifndef _ARRAY_H
#define _ARRAY_H


int64_t array_get_element(int64_t arr[], int64_t ind);
uint8_t array_get_element_c(int64_t arr[], int64_t ind);
double array_get_element_f(double arr[], int64_t ind);

void array_set_element(int64_t arr[], int64_t ind, int64_t el);
void array_set_element_c(int64_t arr[], int64_t ind, int64_t el);
void array_set_element_f(double arr[], int64_t ind, double el);


int64_t _internal_bound_check(int64_t arr[], int64_t ind);

int64_t _internal_get_sizeZintegerArr(int64_t arr[]);
int64_t _internal_get_sizeZcharArr(int64_t arr[]);
int64_t _internal_get_sizeZbyteArr(int64_t arr[]);
int64_t _internal_get_sizeZfloatArr(double arr[]);
int64_t _internal_get_sizeZboolArr(int64_t arr[]);

#endif