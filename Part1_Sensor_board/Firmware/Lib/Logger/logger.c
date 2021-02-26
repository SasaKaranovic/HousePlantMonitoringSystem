// C99 Includes
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>


// Project includes
#include "tinyprintf.h"
#include "logger.h"
#include "usart.h"

// Define uart function that takes char buffer and buffer lenght
#ifndef LOGGER_UART_SEND
#define LOGGER_UART_SEND    HAL_UART_PutStr
#endif

static const char LogLevels[LOG_LAST] = {'T', 'D', 'I', 'W', 'E', 'F', 'O'};


//Static functions
static uint8_t Log_CreateHeader(const char *tag, LogLevel_t level);


#define LOG_BUFFER_MAX_LEN  64
static char gLogBuffer[LOG_BUFFER_MAX_LEN] = {0};


bool Logger_Print(const char* tag, LogLevel_t level, const char* format, ...)
{
    uint8_t hlen = 0;
    uint8_t mlen = 0;
    uint8_t total_len = 0;

    // todo... Check Log level
    hlen = Log_CreateHeader(tag, level);

    // Create log message
    va_list arglist;
    va_start(arglist, format);
    // append the body.
    // mlen = vsnprintf(&gLogBuffer[hlen], LOG_BUFFER_MAX_LEN - hlen, format, arglist);
    mlen = tfp_vsnprintf(&gLogBuffer[hlen], LOG_BUFFER_MAX_LEN - hlen, format, arglist);
    va_end(arglist);
    if (mlen <= 0)
    {
        return false;
    }


    // Calculate total lenght and check for overflow
    if(mlen + hlen >= LOG_BUFFER_MAX_LEN)
    {
        total_len = LOG_BUFFER_MAX_LEN;
    }
    else
    {
        total_len = mlen+hlen;
    }

    // Terminate str with new line
    gLogBuffer[total_len++] = '\r';
    gLogBuffer[total_len++] = '\n';

    LOGGER_UART_SEND((uint8_t *)&gLogBuffer[0], total_len);

    return true;
}


static uint8_t Log_CreateHeader(const char *tag, LogLevel_t level)
{
    int len = 0;

    #if 0
    len = snprintf(&gLogBuffer[0], LOG_BUFFER_MAX_LEN-1, "[%010lu][%c][%s]", HAL_GetTick(), LogLevels[level], tag);
    if(len < 18)
    {
        return 0;
    }
    #else
    // len = snprintf(&gLogBuffer[0], LOG_BUFFER_MAX_LEN-1, "[%c][%s]", LogLevels[level], tag);
    len = tfp_snprintf(&gLogBuffer[0], LOG_BUFFER_MAX_LEN-1, "[%c][%s]", LogLevels[level], tag);

    if(len >= 0)
    {
        return len;
    }
    else
    {
        return 0;
    }
    #endif



    return len;
}
