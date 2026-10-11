#pragma once

/* Log management */

enum {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

#define log_trace(...) llogf(LOG_TRACE,  __VA_ARGS__)
#define log_debug(...) llogf(LOG_DEBUG,  __VA_ARGS__)
#define log_info(...)  llogf(LOG_INFO,   __VA_ARGS__)
#define log_warn(...)  llogf(LOG_WARN,   __VA_ARGS__)
#define log_error(...) llogf(LOG_ERROR,  __VA_ARGS__)
#define log_fatal(...) llogf(LOG_FATAL,  __VA_ARGS__)

static const char *log_level_strings[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "***ERROR",
    "***FATAL"
};

static void llogf(int level, const char *fmt, ...)
{
    va_list ap;

    if (level >= LOG_ERROR)
        fprintf(stderr, "[%-8s]  ", log_level_strings[level]);
    else
        fprintf(stdout, "[%-8s]  ", log_level_strings[level]);

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);

    fflush(stdout);
}
