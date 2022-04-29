#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

bool is_numeric(const char *str);
int rand_in_range(int start, int end);
void random_sleep_ms(int start, int end);
void print_usage();
int min(int first, int second);

#endif
