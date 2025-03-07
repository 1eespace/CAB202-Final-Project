#include <avr/io.h>
#include <avr/interrupt.h>

void tca0_init(void);
void play_tone(uint8_t tone_index);
void stop_tone(void);
