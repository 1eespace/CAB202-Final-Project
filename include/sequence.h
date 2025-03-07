#include <stdint.h>

void next();
void re_seed();

extern volatile uint32_t start_LFSR;
extern volatile uint32_t state_LFSR;
extern volatile uint8_t next_LFSR;
extern volatile uint8_t set_re_seed;
