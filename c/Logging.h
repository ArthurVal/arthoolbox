#pragma once

/**
 *   \file Logging.h
 *   \brief Provide log relative macros that can be disable at compile time
 *          (ATB_LOG_LEVEL).
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*****************************************************************************/
/*                                 LOG LEVEL                                 */
/*****************************************************************************/
// clang-format off
#define ATB_LOG_LEVEL_DEBUG   10
#define ATB_LOG_LEVEL_INFO    20
#define ATB_LOG_LEVEL_WARNING 30
#define ATB_LOG_LEVEL_ERROR   40
#define ATB_LOG_LEVEL_FATAL   50
// clang-format on

/*****************************************************************************/
/*                                   COLOR                                   */
/*****************************************************************************/
/// Building block use to create a sequence identifier
#define ATB_LOG_ANSI_CSI_BEGIN(sequence) "\033[" sequence "m"

/// RESET the terminal
#define ATB_LOG_ANSI_CSI_END "\033[0m"

/// Decorates a string with the ANSI SEQ
#define ATB_LOG_WITH_ANSI(sequence, str)                                     \
  ATB_LOG_ANSI_CSI_BEGIN(sequence) str ATB_LOG_ANSI_CSI_END

// clang-format off
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BLACK          "30"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_RED            "31"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_GREEN          "32"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_YELLOW         "33"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BLUE           "34"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_MAGENTA        "35"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_CYAN           "36"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_WHITE          "37"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BRIGHT_BLACK   "90"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BRIGHT_RED     "91"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BRIGHT_GREEN   "92"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BRIGHT_YELLOW  "93"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BRIGHT_BLUE    "94"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BRIGHT_MAGENTA "95"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BRIGHT_CYAN    "96"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_BRIGHT_WHITE   "97"

#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BLACK          "40"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_RED            "41"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_GREEN          "42"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_YELLOW         "43"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BLUE           "44"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_MAGENTA        "45"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_CYAN           "46"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_WHITE          "47"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BRIGHT_BLACK   "100"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BRIGHT_RED     "101"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BRIGHT_GREEN   "102"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BRIGHT_YELLOW  "103"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BRIGHT_BLUE    "104"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BRIGHT_MAGENTA "105"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BRIGHT_CYAN    "106"
#define ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_BRIGHT_WHITE   "107"
// clang-format on

#if defined(ATB_LOG_DISABLE_COLOR)

#define ATB_LOG_WITH_BG_COLOR(color, str) str
#define ATB_LOG_WITH_FG_COLOR(color, str) str
#define ATB_LOG_WITH_BG_FG_COLOR(bg_color, fg_color, str) str

#else

#define ATB_LOG_WITH_BG_COLOR(color, str)                                    \
  ATB_LOG_WITH_ANSI(ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_##color, str)
#define ATB_LOG_WITH_FG_COLOR(color, str)                                    \
  ATB_LOG_WITH_ANSI(ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_##color, str)
#define ATB_LOG_WITH_BG_FG_COLOR(bg_color, fg_color, str)                    \
  ATB_LOG_WITH_ANSI(ATB_LOG_ANSI_CSI_BASIC_COLOR_BG_##bg_color             \
                      ";" ATB_LOG_ANSI_CSI_BASIC_COLOR_FG_##fg_color,        \
                      str)
#endif

/*****************************************************************************/
/*                                  LOG IMPL                                 */
/*****************************************************************************/
#define ATB_LOG_GET_TIMESTAMP_NS(value)                                      \
  do {                                                                         \
    struct timespec now;                                                       \
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &now)) {                            \
      now.tv_sec = 0;                                                          \
      now.tv_nsec = 0;                                                         \
    }                                                                          \
    value = (typeof(value))now.tv_sec * (typeof(value))1e9 +                   \
            (typeof(value))now.tv_nsec;                                        \
  } while (0)

#define ATB_LOG_IMPL(stream, fmt, ...)                                       \
  do {                                                                         \
    uintmax_t stamp;                                                           \
    ATB_LOG_GET_TIMESTAMP_NS(stamp);                                         \
    fprintf(stream, "[%ju]" fmt "\n", stamp, ##__VA_ARGS__);                   \
  } while (0)

#define ATB_LOG_FG_COLOR_IMPL(stream, color, fmt, ...)                       \
  do {                                                                         \
    uintmax_t stamp;                                                           \
    ATB_LOG_GET_TIMESTAMP_NS(stamp);                                         \
    fprintf(stream, ATB_LOG_WITH_FG_COLOR(color, "[%ju]" fmt "\n"), stamp,   \
            ##__VA_ARGS__);                                                    \
  } while (0)

#define ATB_LOG_BG_COLOR_IMPL(stream, color, fmt, ...)                       \
  do {                                                                         \
    uintmax_t stamp;                                                           \
    ATB_LOG_GET_TIMESTAMP_NS(stamp);                                         \
    fprintf(stream, ATB_LOG_WITH_BG_COLOR(color, "[%ju]" fmt "\n"), stamp,   \
            ##__VA_ARGS__);                                                    \
  } while (0)

#define ATB_LOG_BG_FG_COLOR_IMPL(stream, bg_color, fg_color, fmt, ...)       \
  do {                                                                         \
    uintmax_t stamp;                                                           \
    ATB_LOG_GET_TIMESTAMP_NS(stamp);                                         \
    fprintf(stream,                                                            \
            ATB_LOG_WITH_BG_FG_COLOR(bg_color, fg_color, "[%ju]" fmt "\n"),  \
            stamp, ##__VA_ARGS__);                                             \
  } while (0)

/*****************************************************************************/
/*                               LOG IMPL LEVEL                              */
/*****************************************************************************/

#if !(defined(ATB_LOG_LEVEL))
#define ATB_LOG_LEVEL ATB_LOG_LEVEL_INFO
#endif

#define ATB_LOG_NOOPT(...)                                                   \
  do {                                                                         \
  } while (0)

#if (ATB_LOG_LEVEL <= ATB_LOG_LEVEL_DEBUG)
#define ATB_LOG_DEBUG(fmt, ...)                                              \
  ATB_LOG_FG_COLOR_IMPL(stdout, BRIGHT_GREEN, "[DEBUG  ] " fmt, ##__VA_ARGS__)
#else
#define ATB_LOG_DEBUG(...) ATB_LOG_NOOPT()
#endif

#if (ATB_LOG_LEVEL <= ATB_LOG_LEVEL_INFO)
#define ATB_LOG_INFO(fmt, ...)                                               \
  ATB_LOG_FG_COLOR_IMPL(stdout, BRIGHT_BLUE, "[INFO   ] " fmt, ##__VA_ARGS__)
#else
#define ATB_LOG_INFO(...) ATB_LOG_NOOPT()
#endif

#if (ATB_LOG_LEVEL <= ATB_LOG_LEVEL_WARNING)
#define ATB_LOG_WARNING(fmt, ...)                                            \
  ATB_LOG_FG_COLOR_IMPL(stderr, BRIGHT_YELLOW, "[WARNING] " fmt,             \
                          ##__VA_ARGS__)
#else
#define ATB_LOG_WARNING(...) ATB_LOG_NOOPT()
#endif

#if (ATB_LOG_LEVEL <= ATB_LOG_LEVEL_ERROR)
#define ATB_LOG_ERROR(fmt, ...)                                              \
  ATB_LOG_FG_COLOR_IMPL(stderr, BRIGHT_RED, "[ERROR  ] " fmt, ##__VA_ARGS__)
#else
#define ATB_LOG_ERROR(...) ATB_LOG_NOOPT()
#endif

#if (ATB_LOG_LEVEL <= ATB_LOG_LEVEL_FATAL)
#define ATB_LOG_FATAL(fmt, ...)                                              \
  ATB_LOG_BG_FG_COLOR_IMPL(stderr, RED, BRIGHT_WHITE, "[FATAL  ] " fmt,      \
                             ##__VA_ARGS__);                                   \
  exit(EXIT_FAILURE)
#else
#define ATB_LOG_FATAL(...) exit(EXIT_FAILURE)
#endif

#define ATB_LOG(level, fmt, ...) ATB_LOG_##level(fmt, ##__VA_ARGS__)
