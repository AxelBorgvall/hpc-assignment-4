#ifndef NEWTON_H
#define NEWTON_H

#include "config.h"

int demo_loop(void *arg);

//let it take only thread index. All else will be given through memory
int newton_loop(void*arg);

#endif // NEWTON_H
