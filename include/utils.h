#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>

#define FREE_2D_ARRAY(n, arr) \
  for (int i = 0; i < (n); ++i) {\
    free((arr)[(n)]);\
  }\
  free((arr));

#endif // UTILS_H_
