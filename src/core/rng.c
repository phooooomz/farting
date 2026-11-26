#include "rng.h"
static uint32_t state = 0x2545F491u;
void rng_seed(uint32_t seed) { state = seed ? seed : 0x2545F491u; }
uint32_t rng_next(void) { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
int rng_range(int lo, int hi) { return lo + (int)(rng_next() % (uint32_t)(hi - lo + 1)); }
