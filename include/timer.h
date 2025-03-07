#include <avr/io.h>

void buttons_init(void);
void timer_init(void);

extern volatile uint16_t elapsed_time;
extern volatile uint16_t playback_delay;

extern volatile uint8_t pb_state;
