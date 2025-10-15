#ifndef PPM_WRITER_H
#define PPM_WRITER_H

#include <stdio.h>

#include "config.h"

void init_convergence();
void free_convergence();
void free_attractors();

// PPM file handling
int demo_func(void *arg);

int demo_filewriter(void *arg);

int writer_func(void *arg);


#endif // PPM_WRITER_H
