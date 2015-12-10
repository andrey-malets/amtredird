#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_ARG_SIZE 128

enum {
  CMD_LIST,
  CMD_START,
  CMD_STOP,
  CMD_LAST = CMD_STOP
} cmd_type;

struct command {
  uint8_t type;
  char arg[MAX_ARG_SIZE];
};

enum {
  RES_OK,
  RES_NO_SUCH_CLIENT,
  RES_FAILED,
  RES_NOT_UNDERSTOOD
} result_code;

struct result {
  uint8_t code;
  uint32_t num_clients;
  const struct client *clients;
};

int read_cmd(int socket, struct command *cmd);
int write_result(int socket, const struct result *result);

struct config;
struct result execute(const struct config *config, const struct command *cmd);
