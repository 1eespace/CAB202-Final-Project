#ifndef TYPEDEF_HEADER
#define TYPEDEF_HEADER 1

#define TONE_1 0
#define TONE_2 1
#define TONE_3 2
#define TONE_4 3

typedef enum
{
    CHECK_SCORE,
    RECORD_ENTRY,
    ENTER,
    DISPLAY_ENTRY,
} High_Score_State;

typedef enum
{
    INIT,
    SIMON_QUESTION,
    USER_ANSWER,
    RESULT,
    CHECK_HIGH_SCORE,
} State;

typedef enum
{
    SUCCESS_OR_FAIL,
    DISPLAY_STATUS,
    DISPLAY_SCORE,
} Display_Result_State;

typedef enum
{
    START,
    PLAY,
    SILENT
} Simon_State;

typedef enum
{
    STOP,
    PLAYING,
} User_State;

typedef enum
{
    WAITING_COMMAND,
    WAITING_PAYLOAD,
    WAITING_NAME,
    WAITING_NEWLINE,
} Serial_State;

typedef struct
{
    // 20 characters + null terminator
    char name[21];
    uint16_t score;

} score_entry;

#endif
