#include <stdint.h>
#include "alltypedef.h"

void uart_init(void);
uint8_t uart_getc(void);
void uart_putc(uint8_t c);
void uart_puts(char *string);
void reset_game(void);

extern volatile uint8_t reset;
extern volatile Serial_State SERIAL_STATE;
