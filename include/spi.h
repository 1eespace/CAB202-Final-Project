#include <avr/io.h>
#include <avr/interrupt.h>

void spi_init(void);
void spi_write(uint8_t b);
void display_tone(uint8_t tone_index);
void clear_display(void);
void display_success(void);
void display_fail(void);
void display_score(uint32_t score);

extern volatile uint8_t number_segs[10];

