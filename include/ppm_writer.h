#ifndef PPM_WRITER_H
#define PPM_WRITER_H

#include <stdio.h>

#include "config.h"

void init_convergence();
void free_convergence();
void free_attractors();

// PPM file handling
int writer_func(void *arg);


#endif // PPM_WRITER_H
