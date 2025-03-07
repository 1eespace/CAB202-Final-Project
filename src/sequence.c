#include <avr/io.h>

volatile uint32_t start_LFSR = 0x11355191;
volatile uint32_t state_LFSR = 0x11355191;
volatile uint8_t next_LFSR = 0;
volatile uint8_t set_re_seed = 0;

// Reset LFSR to the seed
void re_seed(void)
{
    state_LFSR = start_LFSR;
}

// Compute and update the next step for the LFSR
void next(void)
{
    uint16_t lsb = state_LFSR & 1;
    state_LFSR >>= 1;

    if (lsb)
    {
        state_LFSR ^= 0xE2023CAB;
    }
    next_LFSR = state_LFSR & 0b11;
}
