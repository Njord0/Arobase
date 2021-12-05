#ifndef _CAST_H
#define _CAST_H

char to_byteZinteger(int64_t i);
int64_t to_intZbyte(char c);
int64_t to_intZchar(char c);
char to_charZinteger(int64_t i);
char to_byteZchar(char c);
char to_charZbyte(char c);

double to_floatZinteger(int64_t i);
int64_t to_intZfloat(double d);


#endif