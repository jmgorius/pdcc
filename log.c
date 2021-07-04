#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static struct {
  log_LockFunc lock_func;
  void *lock_data;
  log_Level level;
  bool quiet;
} log = {};

static const char level_names[][6] = {
    [LOG_LEVEL_DEBUG] = "DEBUG",   [LOG_LEVEL_INFO] = "INFO ",
    [LOG_LEVEL_WARNING] = "WARN ", [LOG_LEVEL_ERROR] = "ERROR",
    [LOG_LEVEL_FATAL] = "FATAL",
};

static const char level_colors[][6] = {
    [LOG_LEVEL_DEBUG] = ANSI_COLOR_CYAN,
    [LOG_LEVEL_INFO] = ANSI_COLOR_GREEN,
    [LOG_LEVEL_WARNING] = ANSI_COLOR_YELLOW,
    [LOG_LEVEL_ERROR] = ANSI_COLOR_RED,
    [LOG_LEVEL_FATAL] = ANSI_COLOR_MAGENTA,
};

static void lock() {
  if (log.lock_func)
    log.lock_func(true, log.lock_data);
}

static void unlock() {
  if (log.lock_func)
    log.lock_func(false, log.lock_data);
}

void log_impl(log_Level level, const char *format, ...) {
  lock();

  if (!log.quiet && level >= log.level) {
    va_list args;
    va_start(args, format);

    time_t now = time(0);
    struct tm *local_time = localtime(&now);
    char time_buf[16];
    time_buf[strftime(time_buf, sizeof(time_buf), "%H:%M:%S", local_time)] = 0;

    printf(ANSI_COLOR_LIGHT_GRAY "%s" ANSI_RESET " %s%s " ANSI_RESET, time_buf,
           level_colors[level], level_names[level]);
    vprintf(format, args);
    putchar('\n');

    va_end(args);
  }

  unlock();
}

void log_set_level(log_Level level) { log.level = level; }

void log_set_quiet(bool quiet) { log.quiet = quiet; }

void log_set_lock_func(log_LockFunc f, void *user_data) {
  log.lock_func = f;
  log.lock_data = user_data;
}
