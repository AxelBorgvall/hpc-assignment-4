#include <complex.h>
#include <math.h> 

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


bool is_bailout(_Complex double z){
  return (fabs(creal(z)) > BAILOUT_MAG || fabs(cimag(z)) > BAILOUT_MAG);
}
bool is_close_to_origin(_Complex double z){
  return (creal(z)*creal(z)+cimag(z)*cimag(z))<BAILOUT_NORM_SQ;
}
int getroot_scan(_Complex double z){
  return 0;//i dont think ill use this  
}
int getroot_arg(_Complex double z) {
    double rad_diff = creal(z)*creal(z) + cimag(z)*cimag(z) - 1.0;
    if (fabs(rad_diff) >= EPS_SQR) return -1; 

    double theta = carg(z);
    if (theta < 0) theta += 2*M_PI;

    int k = (int)(theta * delta_arg_inv + 0.5); 
    if (k >= d) k -= d; // wrap around

    double diff = fabs(theta - k * delta_arg);
    if (diff > M_PI) diff = 2*M_PI - diff; // wrap around

    if (diff > EPSILON * delta_arg) return -1;

    return k;
}


_Complex double newton_update(_Complex double z){
  return newton_c1*z+newton_c2*complex_pow(z,1-d);
}

