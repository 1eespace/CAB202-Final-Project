#include "timer.h"
#include "adc.h"
#include "spi.h"

extern uint8_t segs[];

volatile uint16_t elapsed_time = 0;
volatile uint16_t playback_delay = 0;

volatile uint8_t pb_state = 0xFF;

void buttons_init(void)
{
    // Enable pull-up resistors for PBs
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;

    cli();
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB0 in periodic interrupt mode
    TCB0.CCMP = 3333;                // Set interval for 1ms (3333 clocks @ 3.3 MHz)
    TCB0.INTCTRL = TCB_CAPT_bm;      // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;      // Enable
    sei();
}

void pb_debounce(void)
{
    static uint8_t count0 = 0;
    static uint8_t count1 = 0;
    // Perform an XOR operation to indicate whether there is a change in button input
    // Input value of Port A using PORTA.IN
    uint8_t pb_changed = pb_state ^ PORTA.IN;
    // Update counter
    count1 = (count1 ^ count0) & pb_changed;
    count0 = ~count0 & pb_changed;

    // Update pb_state immediately on falling edge or if 3 counts
    pb_state ^= (count0 & count1);
}

void timer_init(void)
{
    cli();
    TCB1.CTRLA = TCB_CLKSEL_DIV1_gc;
    TCB1.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB0 in periodic interrupt mode
    TCB1.CCMP = 3333;                // Set interval for 1ms (3333 clocks @ 3.3 MHz)
    TCB1.INTCTRL = TCB_CAPT_bm;      // CAPT interrupt enable
    TCB1.CTRLA = TCB_ENABLE_bm;      // Enable
    sei();
}

ISR(TCB1_INT_vect)
{
    playback_delay = adc_result();
    // each ms passed, it increases
    elapsed_time++;

    // reset interrupt flag
    TCB1.INTFLAGS = TCB_CAPT_bm;
}

// Debounced push_buttons handling
ISR(TCB0_INT_vect)
{
    pb_debounce();
    static uint8_t digit = 0;

    if (digit)
    {
        spi_write(segs[0] | (0x01 << 7));
    }
    else
    {
        spi_write(segs[1]);
    }
    digit = !digit;
    // Reset interrupt flag
    TCB0.INTFLAGS = TCB_CAPT_bm;
}
