#ifndef _OPTIONS_H
#define _OPTIONS_H

#include <stdbool.h>

typedef struct Option {
    const char *output_file;
    const char *source_file; 
    bool has_entry; // has entry point '_start' or not
    bool assembly; // generate assembly or not
} Option_t;

extern Option_t *option_g;

void print_usage(const char *arg);
Option_t *options_parse(char **argv, int argc);
char *make_command(char *args[], int argc);

#endif