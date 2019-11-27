#include <stdint.h>
#include <stdbool.h>
#include <smmintrin.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)
#include "base64unpack-simd.h"
