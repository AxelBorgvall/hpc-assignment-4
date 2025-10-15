#include <complex.h>

#include "../include/complex_utils.h"
#include "../include/config.h"


_Complex double complex_pow(_Complex double z, int n) {
  _Complex double result = 1.0 + 0.0 * I;
  _Complex double base = z;

  while (n > 0) {
    if (n & 1) result *= base;
    base *= base;
    n >>= 1;
  }

  return result;
}

/*
double r = cabs(x);
double theta = carg(x);          // -pi .. pi
double k = theta * d / (2*M_PI);
double nearest = round(k);
if (fabs(r - 1.0) < eps && fabs(k - nearest) < eps_angle) {
    // x is close to a root
}
  */


bool is_bailout(_Complex double z){
  return (fabs(creal(z)) > BAILOUT_MAG || fabs(cimag(z)) > BAILOUT_MAG);
}
bool is_close_to_origin(_Complex double z){
  return (creal(z)*creal(z)+cimag(z)*cimag(z))<BAILOUT_NORM_SQ;
}
int getroot(_Complex double z){
  
}
_Complex double newton_update(_Complex double z){
  return newton_c1*z+newton_c2*complex_pow(z,1-d);
}

