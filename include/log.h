#ifndef LOG_H
#define LOG_H

#include "utils.h"
#define STATUS_WARN "WARNING"
#define STATUS_ERROR "ERROR"
#define STATUS_INFO "INFO"

#ifdef DEBUG
extern void logger_log(const char *status, const char *fmt, ...);
extern void logger_log_state(void);

#define LOG_INFO(fmt, ...)                                                     \
  logger_log(STATUS_INFO, fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARN(fmt, ...)                                                     \
  logger_log(STATUS_WARN, fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_ERROR(fmt, ...)                                                    \
  logger_log(STATUS_ERROR, fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_PANIC(msg)                                                         \
  do {                                                                         \
    LOG_ERROR((msg));                                                          \
    PANIC((msg));                                                              \
  } while (0)

#define LOG_STATE() logger_log_state()
#else
#define LOG_INFO(fmt, ...) (void)0
#define LOG_WARN(fmt, ...) (void)0
#define LOG_ERROR(fmt, ...) (void)0
#define LOG_PANIC(msg) PANIC((msg))
#define LOG_STATE() (void)0
#endif

#endif
