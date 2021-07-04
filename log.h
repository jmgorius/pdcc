//===----------------------------------------------------------------------===//
// log - Thread-safe logging library
//
// This is free and unencumbered software released into the public domain.
//
// This library provides a set of thread-safe logging macros. Colored output is
// enabled by default and can be disabled by defining LOG_DISABLE_COLORS before
// including this header.
//
// NOTE: The log_set_* functions are not thread-safe.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_LOG_H
#define INCLUDED_LOG_H

#ifndef LOG_DISABLE_COLORS
#include "ansi-codes.h"
#else
#define ANSI_RESET ""
#define ANSI_COLOR_RED ""
#define ANSI_COLOR_GREEN ""
#define ANSI_COLOR_YELLOW ""
#define ANSI_COLOR_MAGENTA ""
#define ANSI_COLOR_CYAN ""
#define ANSI_COLOR_LIGHT_GRAY ""
#endif

#include "compiler.h"

#include <stdbool.h>

typedef enum {
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_FATAL
} log_Level;

#define log_debug(...) log_impl(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define log_info(...) log_impl(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_warning(...) log_impl(LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_error(...) log_impl(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_fatal(...) log_impl(LOG_LEVEL_FATAL, __VA_ARGS__)

PRINTF_FORMAT(2, 3)
void log_impl(log_Level level, const char *format, ...);

void log_set_level(log_Level level);
void log_set_quiet(bool quiet);

typedef void (*log_LockFunc)(bool lock, void *data);
void log_set_lock_func(log_LockFunc f, void *user_data);

#endif // INCLUDED_LOG_H
