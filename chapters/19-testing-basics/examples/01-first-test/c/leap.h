#ifndef LEAP_H
#define LEAP_H

#include <stdbool.h>

// Whether year is a leap year in the Gregorian calendar: one divisible by 4,
// except for centuries, which must be divisible by 400.
bool is_leap_year(int year);

#endif
