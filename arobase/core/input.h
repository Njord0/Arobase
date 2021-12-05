#ifndef _INPUT_H
#define _INPUT_H

#include <stdint.h>

int64_t input_integer();
char input_char();
char input_byte();
double input_float();

ssize_t _internal_read(char *ptr, unsigned int len);
void _internal_flush();

#endif