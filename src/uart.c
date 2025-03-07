#include "buzzer.h"
#include "timer.h"
#include "sequence.h"
#include "spi.h"
#include "alltypedef.h"
#include "uart.h"

volatile uint8_t reset = 0;
volatile Serial_State SERIAL_STATE = WAITING_COMMAND;
extern volatile State STATE;
extern volatile User_State USER_STATE;
extern volatile uint8_t pb_released;
extern volatile uint8_t user_input_index;
extern volatile uint8_t save_entry;
extern volatile char name_entry[20];
extern volatile uint8_t current_input_index;
extern uint32_t periods[4];

void uart_init(void)
{
    PORTB.DIRSET = PIN2_bm;
    // 9600 baud
    USART0.BAUD = 1389;

    // Enable receive complete interrupt
    USART0.CTRLA = USART_RXCIE_bm;
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}

uint8_t uart_getc(void)
{
    while (!(USART0.STATUS & USART_RXCIF_bm))
        ; // Wait for data
    return USART0.RXDATAL;
}

void uart_putc(uint8_t c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
        ; // Wait for TXDATA empty
    USART0.TXDATAL = c;
}

void uart_puts(char *string)
{
    while (*string != '\0')
    {
        uart_putc(*string);
        string++;
    }
}

// Reset frequencies to default
void reset_game(void)
{
    stop_tone();
    clear_display();
    reset = 1;
}

uint8_t hex_char_to_int(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else
        // Invalid
        return 16;
}

ISR(USART0_RXC_vect)
{
    static uint8_t chars_received = 0;
    static uint32_t payload = 0;
    static uint8_t payload_valid = 1;
    // Read data from serial
    uint8_t rx_data = USART0.RXDATAL;

    switch (SERIAL_STATE)
    {
    case WAITING_COMMAND:
        switch (rx_data)
        {
        case '1':
        case 'q':
        case '2':
        case 'w':
        case '3':
        case 'e':
        case '4':
        case 'r':
            if (STATE == USER_ANSWER && USER_STATE == STOP)
            {

                // Same as button 1 pressed
                if (rx_data == '1' || rx_data == 'q')
                {
                    play_tone(TONE_1);
                    display_tone(TONE_1);
                    pb_released = 1;
                    elapsed_time = 0;
                    user_input_index = 0;
                    USER_STATE = PLAYING;
                }

                // Same as button 2 pressed
                else if (rx_data == '2' || rx_data == 'w')
                {
                    play_tone(TONE_2);
                    display_tone(TONE_2);
                    pb_released = 1;
                    elapsed_time = 0;
                    user_input_index = 1;
                    USER_STATE = PLAYING;
                }

                // Same as button 3 pressed
                else if (rx_data == '3' || rx_data == 'e')
                {
                    play_tone(TONE_3);
                    display_tone(TONE_3);
                    pb_released = 1;
                    elapsed_time = 0;
                    user_input_index = 2;
                    USER_STATE = PLAYING;
                }

                // Same as button 4 pressed
                else if (rx_data == '4' || rx_data == 'r')
                {
                    play_tone(TONE_4);
                    display_tone(TONE_4);
                    pb_released = 1;
                    elapsed_time = 0;
                    user_input_index = 3;
                    USER_STATE = PLAYING;
                }
            }
            break;
        // INC FREQ
        case ',':
        case 'k':
            periods[2] /= 2;
            periods[3] /= 2;
            break;
        // DEC FREQ
        case '.':
        case 'l':
            periods[0] *= 2;
            periods[1] *= 2;
            break;
        case '0':
        case 'p':
            reset_game();
            break;
        case '9':
        case 'o':
            if (rx_data == '9' || rx_data == 'o')
            {
                payload_valid = 1;
                chars_received = 0;
                payload = 0;
                SERIAL_STATE = WAITING_PAYLOAD;
                break;
            }

        default:
            break;
        }
        break;

    case WAITING_PAYLOAD:
    {
        uint8_t parsed_result = hex_char_to_int((char)rx_data);

        if (parsed_result != 16)
        {
            payload = (payload << 4) | parsed_result;
            chars_received++;
        }
        else
        {
            // Invalid character,
            // reset payload_valid and chars_received
            payload_valid = 0;
            chars_received = 0;
        }

        if (chars_received == 8)
        {
            if (payload_valid)
            {
                // Set the new seed
                start_LFSR = payload;
                set_re_seed = 1;
            }

            // Reset state to WAITING_COMMAND regardless of payload validity
            SERIAL_STATE = WAITING_COMMAND;
        }
        break;
    }

    case WAITING_NAME:
    {

        static uint8_t characters_received = 0;
        uart_putc(rx_data);
        // ENTER pressed
        if (rx_data == '\r')
        {
            save_entry = 1;
            SERIAL_STATE = WAITING_NEWLINE;
        }
        else if (rx_data == '\n')
        {
            save_entry = 1;
            SERIAL_STATE = WAITING_COMMAND;
        }
        else
        {
            name_entry[current_input_index] = rx_data;
            current_input_index++;
            elapsed_time = 0;
        }

        if (++characters_received == 20)
        {
            save_entry = 1;
        }
        break;
    }

    case WAITING_NEWLINE:
    {
        // Return to WAITING_NAME state
        SERIAL_STATE = WAITING_NAME;
        break;
    }

    default:
        break;
    }
}
