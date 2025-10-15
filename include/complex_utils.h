#ifndef COMPLEX_UTILS_H
#define COMPLEX_UTILS_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"

void generate_block_points(uint64_t start_idx, uint64_t end_idx, _Complex double *points);
// functions for complex doubles
_Complex double complex_pow(_Complex double z, int n);
bool is_bailout(_Complex double z);
bool is_close_to_origin(_Complex double z);
int getroot_scan(_Complex double z);
int getroot_arg(_Complex double z);

_Complex double newton_update(_Complex double z);//get next step

// Add more for higher degrees as needed
#endif // COMPLEX_UTILS_H
