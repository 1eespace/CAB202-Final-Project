#include <avr/interrupt.h>
#include "alltypedef.h"

#define SEGS_EF 0b00111110
#define SEGS_BC 0b01101011
#define SEGS_OFF 0b01111111

#define SEGS_SUCCESS 0b00000000
#define SEGS_FAIL 0b01110111

// Segs initially off
volatile uint8_t segs[2] = {SEGS_OFF, SEGS_OFF};

// The number using Segs for displaying the score
volatile uint8_t number_segs[10] = {
    0x08, 0x6B, 0x44, 0x41, 0x23, 0x11, 0x10, 0x4B, 0x00, 0x01};

void spi_init(void)
{
    cli();
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc; // SPI pins on PC0-3
    PORTC.DIRSET = (PIN0_bm | PIN2_bm);       // Set SCK (PC0) and MOSI (PC2) as outputs
    PORTA.OUTSET = PIN1_bm;                   // DISP LATCH as output, initially high
    PORTA.DIRSET = PIN1_bm;

    SPI0.CTRLA = SPI_MASTER_bm;  // Master, /4 prescaler, MSB first  
    SPI0.CTRLB = SPI_SSD_bm;     // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;    // Interrupt enable
    SPI0.CTRLA |= SPI_ENABLE_bm; // Enable
    sei();
}

// Displaying tone on 7-segmens display
// based on current sequence digit
void display_tone(uint8_t tone_index)
{
    switch (tone_index)
    {
    case TONE_1:
        segs[0] = SEGS_EF;
        segs[1] = SEGS_OFF;
        break;
    case TONE_2:
        segs[0] = SEGS_BC;
        segs[1] = SEGS_OFF;
        break;
    case TONE_3:
        segs[0] = SEGS_OFF;
        segs[1] = SEGS_EF;
        break;
    case TONE_4:
        segs[0] = SEGS_OFF;
        segs[1] = SEGS_BC;
        break;
    default:
        break;
    }
}

// Displaying results success or fail
void display_success(void)
{
    segs[0] = SEGS_SUCCESS;
    segs[1] = SEGS_SUCCESS;
}

void display_fail(void)
{
    segs[0] = SEGS_FAIL;
    segs[1] = SEGS_FAIL;
}

// Displaying score (method)
void display_score(uint32_t number_count)
{
    uint32_t digit, tens = 0;
    // If number is (0~9) then, display same as number
    digit = number_count;
    if (digit < 10)
    {
        segs[0] = SEGS_OFF;
        segs[1] = number_segs[digit];
        return;
    }
    // But, number is higher than 9
    // ex> 109 then '09' is displayed
    while (digit > 9)
    {
        digit -= 10;
        tens++;
    }

    segs[0] = number_segs[tens];
    segs[1] = number_segs[digit];
}

void clear_display(void)
{
    segs[0] = SEGS_OFF;
    segs[1] = SEGS_OFF;
}

void spi_write(uint8_t b)
{
    // DATA register used for both Tx and Rx
    SPI0.DATA = b;
}

ISR(SPI0_INT_vect)
{
    // Rising edge DISP LATCH
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;
    SPI0.INTFLAGS = SPI_IF_bm;
}
