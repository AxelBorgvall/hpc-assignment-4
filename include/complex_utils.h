#ifndef COMPLEX_UTILS_H
#define COMPLEX_UTILS_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"

// Optimized complex operations for Newton iterations
_Complex double complex_pow(_Complex double z, uint8_t n);
bool is_bailout(_Complex double z);
bool is_close_to_origin(_Complex double z);
int getroot(_Complex double z, uint8_t d);//return -1 of not near a root
// Degree-specific optimizations (if needed)
_Complex double newton_update(_Complex double z,uint8_t d);//get next step

// Add more for higher degrees as needed
#endif // COMPLEX_UTILS_H
