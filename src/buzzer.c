#include "spi.h"

// Week12 Buzzer Frequency
/*
Note:
E (high) 368hz
C# 309hz
A 491hz
E (low) 184hz
*/
#define E_HIGH 9058   // E High
#define C_SHARP 10787 // C#
#define A 6789        // A
#define E_LOW 18116   // E Low
uint32_t periods[4] = {
    E_HIGH,
    C_SHARP,
    A,
    E_LOW,
};

/*
Initialize TCA0 to generate a PWM signal for the buzzer.
*/
void tca0_init(void)
{
    // Enable Buzzer as an output
    PORTB.DIRSET = PIN0_bm;

    // TCA clock uses system clock at 3.3mhz
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;
    // Configure TCA0 in single slope mode and set CMP0EN bit
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

    // Enable TCA0
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

void play_tone(uint8_t tone_index)
{
    TCA0.SINGLE.PERBUF = periods[tone_index];
    TCA0.SINGLE.CMP0BUF = TCA0.SINGLE.PERBUF >> 1;

    // Activate PWM output
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0_bm;
}

void stop_tone(void)
{
    // Deactivate PWM output
    TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP0_bm;
}