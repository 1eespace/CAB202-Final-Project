#include <avr/io.h>
#include <stdio.h>
#include <string.h>

#include "adc.h"
#include "buzzer.h"
#include "sequence.h"
#include "spi.h"
#include "timer.h"
#include "alltypedef.h"
#include "uart.h"

volatile State STATE = INIT;
volatile High_Score_State HIGHSCORE = CHECK_SCORE;
volatile User_State USER_STATE = STOP;
volatile Simon_State SIMON_STATE = SILENT;
volatile Display_Result_State RESULT_STATE = START;

volatile int32_t track_tone = 0;
volatile uint8_t pb_released = 0;

volatile uint16_t sequence_length = 1;
volatile uint32_t sequence_matched = 1;

volatile uint8_t user_input_index = 0;
volatile uint8_t save_entry = 0;
char name_entry[20];
volatile uint8_t current_input_index = 0;

const uint8_t MAX_HIGHSCORE = 5;
score_entry high_score[5] = {0};
uint32_t user_score = 0;

int stdio_putchar(char c, FILE *stream)
{
    uart_putc(c);
    return c;
}

void stdio_init(void)
{

    static FILE stdio = FDEV_SETUP_STREAM(stdio_putchar, NULL, _FDEV_SETUP_WRITE);

    stdout = &stdio;
    stdin = &stdio;
}

// Check whether the sequence matches between simon and user
void check_sequence_matched(uint8_t tone_digit, uint8_t LFSR_digit)
{
    if (sequence_matched)
    {
        sequence_matched = tone_digit == LFSR_digit;
    }
}

// Checking the score is in top five
uint8_t in_top_five(uint16_t score)
{
    for (int8_t index = 0; index < MAX_HIGHSCORE; index++)
    {
        if (score > high_score[index].score)
        {
            return 1;
        }
    }
    return 0;
}

// Add name and score to the highscore top 5 list
void add_highscore(volatile char *name, uint16_t score)
{
    // index = i
    uint8_t i;
    for (i = 0; i < MAX_HIGHSCORE; i++)
    {
        if (score > high_score[i].score)
        {
            break;
        }
    }

    // Shift scores that are lower than given score
    for (int8_t j = MAX_HIGHSCORE - 1; j > i; j--)
    {
        high_score[j] = high_score[j - 1];
    }

    strncpy(high_score[i].name, name, current_input_index); // 'strncpy' discards 'volatile' => warning

    high_score[i].name[current_input_index] = '\0';
    high_score[i].score = score;
}

// Display score list on the monitor
void display_highscore_list(void)
{
    printf("\n");
    for (uint8_t i = 0; i < 5; ++i)
    {
        if (high_score[i].score != 0)
        {
            printf("%s", high_score[i].name);
            printf(" %d", high_score[i].score);
            printf("\n");
        }
    }
}

int main(void)
{
    cli();
    adc_init();
    buttons_init();
    stop_tone();
    tca0_init();
    timer_init();
    uart_init();
    spi_init();
    stdio_init();
    sei();

    // Push button states
    uint8_t pb_c = 0xFF;
    uint8_t pb_p = 0xFF;
    uint8_t pb_changed, pb_falling, pb_rising;
    uint16_t half_playback_delay = 0;
    playback_delay = adc_result();

    while (1)
    {
        // Game play state machine encapsulates all possible state of the game
        switch (STATE)
        {
        case INIT:
            sequence_length = 1;
            STATE = SIMON_QUESTION;
            break;

        case SIMON_QUESTION:
            if (set_re_seed)
            {
                re_seed();
                set_re_seed = 0;
            }
            // reset to initial state
            state_LFSR = start_LFSR;

            track_tone = 0;
            SIMON_STATE = START;

            // Using loop for each player turn
            while (1)
            {
                half_playback_delay = playback_delay >> 1;

                // Stop Simon's turn upon RESET
                // reach the end of Simon's sequence
                if (reset)
                    break;
                if (track_tone >= sequence_length)
                    break;

                // State machine helps the different states during Simon sequence
                switch (SIMON_STATE)
                {
                case START:
                    // Starting simon's sequence
                    next();
                    elapsed_time = 0;
                    // disable playback update during play
                    play_tone(next_LFSR);
                    display_tone(next_LFSR);

                    SIMON_STATE = PLAY;
                    break;
                case PLAY:
                    if (elapsed_time >= half_playback_delay)
                    {
                        stop_tone();
                        clear_display();
                        SIMON_STATE = SILENT;
                    }
                    break;
                case SILENT:
                    if (elapsed_time >= playback_delay)
                    {
                        track_tone++;
                        SIMON_STATE = START;
                    }
                    break;
                default:
                    break;
                }
            }

            if (reset)
            {
                STATE = INIT;
                reset = 0;
                break;
            }

            STATE = USER_ANSWER;
            break;

        case USER_ANSWER:
            USER_STATE = STOP;
            track_tone = 0;
            sequence_matched = 1;
            state_LFSR = start_LFSR;

            while (1)
            {
                if (reset)
                {
                    break;
                }
                // Stop turn if user press different button
                if (track_tone >= sequence_length || sequence_matched == 0)
                {
                    break;
                }

                // Previous debounced state
                pb_p = pb_c;
                pb_c = pb_state;

                // Compare edge & prev/new state
                pb_changed = pb_p ^ pb_c;
                pb_rising = pb_changed & pb_c;
                pb_falling = pb_changed & pb_p;

                // This state machine encapsulates the different states
                // that happen during human player's turn
                switch (USER_STATE)
                {
                case STOP:

                    if (pb_falling & PIN4_bm) // PB1
                    {
                        // disable playback time update when pressed
                        play_tone(TONE_1);
                        display_tone(TONE_1);
                        pb_released = 0;
                        elapsed_time = 0;
                        user_input_index = 0;
                        USER_STATE = PLAYING;
                    }
                    else if (pb_falling & PIN5_bm) // PB2
                    {
                        play_tone(TONE_2);
                        display_tone(TONE_2);
                        pb_released = 0;
                        elapsed_time = 0;
                        user_input_index = 1;
                        USER_STATE = PLAYING;
                    }
                    else if (pb_falling & PIN6_bm) // PB3
                    {
                        play_tone(TONE_3);
                        display_tone(TONE_3);
                        pb_released = 0;
                        elapsed_time = 0;
                        user_input_index = 2;
                        USER_STATE = PLAYING;
                    }
                    else if (pb_falling & PIN7_bm) // PB4
                    {
                        play_tone(TONE_4);
                        display_tone(TONE_4);
                        pb_released = 0;
                        elapsed_time = 0;
                        user_input_index = 3;
                        USER_STATE = PLAYING;
                    }
                    break;

                case PLAYING:
                    // If Not released
                    if (!pb_released)
                    {
                        // Now if button is released
                        if (pb_rising & PIN4_bm && user_input_index == 0)
                            pb_released = 1;
                        else if (pb_rising & PIN5_bm && user_input_index == 1)
                            pb_released = 1;
                        else if (pb_rising & PIN6_bm && user_input_index == 2)
                            pb_released = 1;
                        else if (pb_rising & PIN7_bm && user_input_index == 3)
                            pb_released = 1;
                    }
                    // Released
                    else

                    {
                        if (elapsed_time >= (playback_delay >> 1) && pb_released)
                        {
                            stop_tone();
                            clear_display();
                            next();
                            check_sequence_matched(user_input_index, next_LFSR);
                            track_tone++;

                            // Register another user's input
                            USER_STATE = STOP;
                        }
                    }
                    break;
                default:
                    break;
                }
            }

            if (reset)
            {
                STATE = INIT;
                reset = 0;
                break;
            }

            STATE = RESULT;
            break;

        case RESULT:
            RESULT_STATE = SUCCESS_OR_FAIL;

            // Logic in displaying result state
            while (1)
            {
                if (reset)
                {
                    break;
                }

                // Stop the loop when user lost
                if (STATE == SIMON_QUESTION || STATE == CHECK_HIGH_SCORE)
                    break;

                switch (RESULT_STATE)
                {
                // Check whether user passed or failed
                case SUCCESS_OR_FAIL:
                    // SUCCESS
                    if (sequence_matched)
                    {
                        elapsed_time = 0;
                        display_success();
                        printf("SUCCESS\n");
                        user_score = sequence_length;
                        sequence_length++;
                        RESULT_STATE = DISPLAY_STATUS;
                    }
                    // If simon's question and the user's answer are different
                    // GAME OVER
                    else
                    {
                        elapsed_time = 0;
                        display_fail();
                        printf("GAME OVER\n");
                        user_score = sequence_length;
                        sequence_length = 1;
                        start_LFSR = state_LFSR;
                        RESULT_STATE = DISPLAY_STATUS;
                    }

                    break;

                case DISPLAY_STATUS:
                    if (elapsed_time >= playback_delay)
                    {
                        elapsed_time = 0;
                        printf("%lu\n", user_score);
                        RESULT_STATE = DISPLAY_SCORE;
                    }
                    break;

                case DISPLAY_SCORE:
                    if (elapsed_time >= playback_delay)
                    {
                        clear_display();
                    }
                    if (sequence_length == 1)
                    {
                        // Display the user's score when game over
                        display_score(user_score);
                        STATE = CHECK_HIGH_SCORE;
                        HIGHSCORE = CHECK_SCORE;
                    }
                    else
                    {
                        // Restart the game
                        STATE = SIMON_QUESTION;
                    }

                    break;
                default:
                    break;
                }
            }

            if (reset)
            {
                STATE = INIT;
                playback_delay = adc_result();
                reset = 0;
                break;
            }
            break;

        case CHECK_HIGH_SCORE:
        {
            // Different states when user need to enter their scores
            switch (HIGHSCORE)
            {
            case CHECK_SCORE:
                if (!in_top_five(user_score))
                {
                    // Restart the game
                    STATE = SIMON_QUESTION;
                    SERIAL_STATE = WAITING_COMMAND;
                    break;
                }

                // Otherwise, start recording user's name
                SERIAL_STATE = WAITING_NAME;
                HIGHSCORE = RECORD_ENTRY;
                current_input_index = 0;
                printf("Enter name:");
                break;

            case RECORD_ENTRY:
                elapsed_time = 0;
                HIGHSCORE = ENTER;
                break;

            case ENTER:
                // Check whether username is blank after 5 seconds
                if (current_input_index == 0)
                {
                    if (elapsed_time > 5000)
                    {
                        add_highscore("", user_score);
                        HIGHSCORE = DISPLAY_ENTRY;
                    }
                }
                else
                {
                    // If no input after 5 seconds
                    if (elapsed_time > 5000 || save_entry)
                    {
                        HIGHSCORE = DISPLAY_ENTRY;
                        add_highscore((char *)name_entry, user_score);
                        save_entry = 0;
                        break;
                    }
                }

                break;
            case DISPLAY_ENTRY:
                display_highscore_list();
                STATE = SIMON_QUESTION;
                SERIAL_STATE = WAITING_COMMAND;
                break;
            };
        }
        break;

        default:
            break;
        }
    }
}
