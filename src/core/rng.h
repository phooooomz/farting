#ifndef ECLIPSE_RNG_H
#define ECLIPSE_RNG_H
#include <stdint.h>
void     rng_seed(uint32_t seed);
uint32_t rng_next(void);
int      rng_range(int lo, int hi);   /* inclusive */
#endif
