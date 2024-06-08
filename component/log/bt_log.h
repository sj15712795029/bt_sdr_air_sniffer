#ifndef BT_LOG_H_H_H
#define BT_LOG_H_H_H

#include "bt_config.h"

/* Define trace levels */
#define BT_TRACE_LEVEL_NONE    0U          /* No trace messages to be generated    */
#define BT_TRACE_LEVEL_ERROR   1U          /* Error condition trace messages       */
#define BT_TRACE_LEVEL_WARNING 2U          /* Warning condition trace messages     */
#define BT_TRACE_LEVEL_INFO    3U          /* API traces                           */
#define BT_TRACE_LEVEL_DEBUG   4U          /* Full debug messages                  */
#define BT_TRACE_LEVEL_VERBOSE 5U          /* Verbose debug messages               */

#if BT_TRACE_COLOR > 0
#define LOG_COLOR_YELLOW "\033[40;33m"
#define LOG_COLOR_RED "\033[40;31m"
#define LOG_COLOR_BLUE "\033[40;36m"
#define LOG_COLOR_PURPLE "\033[40;35m"
#define LOG_COLOR_RESET "\033[0m"
#else
#define LOG_COLOR_YELLOW 
#define LOG_COLOR_RED 
#define LOG_COLOR_BLUE 
#define LOG_COLOR_PURPLE 
#define LOG_COLOR_RESET 
#endif

/******************* ZMP debug macro ***********************************/
#define ZMP_TRACE_ERROR(fmt,...)                                      \
  {                                                                \
    if (ZMP_TRACE_LEVEL >= BT_TRACE_LEVEL_ERROR)               \
      BT_DEBUG(LOG_COLOR_BLUE "[ ZMP ] " LOG_COLOR_RESET LOG_COLOR_RED "[%s:%d] " fmt LOG_COLOR_RESET "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
  }

#define ZMP_TRACE_WARNING(fmt,...)                                      \
  {                                                                \
    if (ZMP_TRACE_LEVEL >= BT_TRACE_LEVEL_WARNING)               \
      BT_DEBUG(LOG_COLOR_BLUE "[ ZMP ] " LOG_COLOR_RESET LOG_COLOR_YELLOW "[%s:%d] " fmt LOG_COLOR_RESET "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
  }

#define ZMP_TRACE_DEBUG(fmt,...)                                      \
  {                                                                \
    if (ZMP_TRACE_LEVEL >= BT_TRACE_LEVEL_DEBUG)               \
      BT_DEBUG(LOG_COLOR_BLUE "[ ZMP ] " LOG_COLOR_RESET "[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
  }

#define ZMP_TRACE_INFO(fmt,...)                                      \
  {                                                                \
    if (ZMP_TRACE_LEVEL >= BT_TRACE_LEVEL_INFO)               \
      BT_DEBUG(LOG_COLOR_BLUE "[ ZMP ] " LOG_COLOR_RESET "[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
  }

/******************* LL preparse debug macro ***********************************/
#define LL_PREPARSE_TRACE_ERROR(fmt,...)                                      \
{                                                                \
  if (LL_PREPARSE_TRACE_LEVEL >= BT_TRACE_LEVEL_ERROR)               \
    BT_DEBUG(LOG_COLOR_BLUE "[ LLP ] " LOG_COLOR_RESET LOG_COLOR_RED "[%s:%d] " fmt LOG_COLOR_RESET "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

#define LL_PREPARSE_TRACE_WARNING(fmt,...)                                      \
{                                                                \
  if (LL_PREPARSE_TRACE_LEVEL >= BT_TRACE_LEVEL_WARNING)               \
    BT_DEBUG(LOG_COLOR_BLUE "[ LLP ] " LOG_COLOR_RESET LOG_COLOR_YELLOW "[%s:%d] " fmt LOG_COLOR_RESET "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

#define LL_PREPARSE_TRACE_DEBUG(fmt,...)                                      \
{                                                                \
  if (LL_PREPARSE_TRACE_LEVEL >= BT_TRACE_LEVEL_DEBUG)               \
    BT_DEBUG(LOG_COLOR_BLUE "[ LLP ] " LOG_COLOR_RESET "[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

#define LL_PREPARSE_TRACE_INFO(fmt,...)                                      \
{                                                                \
  if (LL_PREPARSE_TRACE_LEVEL >= BT_TRACE_LEVEL_INFO)               \
    BT_DEBUG(LOG_COLOR_BLUE "[ LLP ] " LOG_COLOR_RESET "[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

/******************* LL parse debug macro ***********************************/
#define LL_TRACE_ERROR(fmt,...)                                      \
{                                                                \
  if (LL_TRACE_LEVEL >= BT_TRACE_LEVEL_ERROR)               \
    BT_DEBUG(LOG_COLOR_BLUE "[ LL ] " LOG_COLOR_RESET LOG_COLOR_RED "[%s:%d] " fmt LOG_COLOR_RESET "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

#define LL_TRACE_WARNING(fmt,...)                                      \
{                                                                \
  if (LL_TRACE_LEVEL >= BT_TRACE_LEVEL_WARNING)               \
    BT_DEBUG(LOG_COLOR_BLUE "[ LL ] " LOG_COLOR_RESET LOG_COLOR_YELLOW "[%s:%d] " fmt LOG_COLOR_RESET "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

#define LL_TRACE_DEBUG(fmt,...)                                      \
{                                                                \
  if (LL_TRACE_LEVEL >= BT_TRACE_LEVEL_DEBUG)               \
    BT_DEBUG(LOG_COLOR_BLUE "[ LL ] " LOG_COLOR_RESET "[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

#define LL_TRACE_INFO(fmt,...)                                      \
{                                                                \
  if (LL_TRACE_LEVEL >= BT_TRACE_LEVEL_INFO)               \
    BT_DEBUG(LOG_COLOR_BLUE "[ LL ] " LOG_COLOR_RESET "[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

#define BT_HEX_TRACE_DEBUG(fmt,...)                                      \
{                                                                \
  if (BT_HEX_TRACE_LEVEL > BT_TRACE_LEVEL_NONE)               \
    BT_DEBUG(fmt,##__VA_ARGS__); \
}

#define MAX_COL 16
#define SHOW_LINE_SIZE 16
static inline void bt_hex_dump(uint8_t *data, uint32_t len) {
    uint32_t line;
    uint32_t curline = 0;
    uint32_t curcol = 0;
    char showline[SHOW_LINE_SIZE];
    uint32_t data_pos = 0;

    if(len % MAX_COL) {
        line = len / MAX_COL + 1;
    } else {
        line = len / MAX_COL;
    }

    for(curline = 0; curline < line; curline++) {
        sprintf(showline, "%08xh:", curline * MAX_COL);
        BT_HEX_TRACE_DEBUG("%s", showline);
        for(curcol = 0; curcol < MAX_COL; curcol++) {
            if(data_pos < len) {
                BT_HEX_TRACE_DEBUG("%02x ", data[data_pos]);
                data_pos++;
            } else {
                break;
            }
        }
        BT_HEX_TRACE_DEBUG("\n");
    }
}


#endif