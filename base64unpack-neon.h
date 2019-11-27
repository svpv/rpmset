#include <stdint.h>
#include <stdbool.h>
#include <arm_neon.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)
#include "base64unpack-simd.h"
