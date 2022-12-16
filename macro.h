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

#define IMR_RETRY(times, res, host, cmd, ...) \
  { \
    int stop = 0; \
    for (size_t _try = 0; !stop && _try != times; ++_try) { \
      res = cmd(__VA_ARGS__); \
      switch (res) { \
      case IMR_RES_TIMEOUT: \
      case IMR_RES_SOCKET_ERROR: \
        display_error(host, #cmd, res); \
        break; \
      case IMR_RES_OK: \
        stop = 1; \
        break; \
      default: \
        display_error(host, #cmd, res); \
        stop = 1; \
        break; \
      } \
    } \
  }

#define IMR_HANDLE_CLOSED(res, act) \
  switch (res) { \
  case IMR_RES_OK: \
  case IMR_RES_SESSION_CLOSED: \
    break; \
  default: \
    act; \
    break; \
  }
