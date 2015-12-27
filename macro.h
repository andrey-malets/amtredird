#pragma once

#define GOTO_WITH(label, retval, value) \
  do { \
    (retval) = (value); \
    goto label; \
  } while (0)

#define SYSCALL(expr) ((expr) != -1)

#define ERROR(msg) \
  fputs(msg "\n", stderr)

#define PERROR1(msg, arg) \
  do { \
    fputs(msg " ", stderr); \
    perror(arg); \
  } while (0)

#define CHECK(cond, alert, act) \
  do { \
    if (!(cond)) {\
      alert; \
      act; \
    } \
  } while (0)

#define IMR_CHECK(act, host, cmd, ...) \
  do { \
    IMRResult res = cmd(__VA_ARGS__); \
    if (res != IMR_RES_OK) { \
      display_error(host, #cmd, res); \
      act; \
    } \
  } while (0)

#define IMR_RETRY_ON_TIMEOUT(times, act, host, cmd, ...) \
  for (size_t _try = 0; _try != times; ++_try) { \
    IMRResult res = cmd(__VA_ARGS__); \
    if (res == IMR_RES_TIMEOUT) { \
      display_error(host, #cmd, res); \
      continue; \
    } \
    if (res != IMR_RES_OK) { \
      display_error(host, #cmd, res); \
      act; \
    } \
    break; \
  }
