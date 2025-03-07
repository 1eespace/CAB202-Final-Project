#include <avr/io.h>

void adc_init(void)
{
    ADC0.CTRLA = ADC_ENABLE_bm;
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp | ADC_REFSEL_VDD_gc);
    ADC0.CTRLE = 64;
    ADC0.CTRLF = ADC_FREERUN_bm | ADC_LEFTADJ_bm;

    // Use POT AIN2
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;

    // Single ended 8-bit conversion
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;
}

uint16_t adc_result()
{
    // The intermediate multiplication result is stored in a 32-bit integer
    uint32_t adc0_result = ADC0.RESULT * 1750;

    // Divide by 255 to get our desired scaled value.
    uint16_t result = 250 + (adc0_result >> 8);
    return result;
}
