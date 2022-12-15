#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_ARG_SIZE 128

struct command {
  uint8_t type;
  char arg[MAX_ARG_SIZE];
};

struct result {
  uint8_t code;
  uint32_t num_clients;
  const struct client *clients;
};

int read_cmd(int socket, struct command *cmd);
int write_result(int socket, const struct result *result);

struct config;
struct result execute(const struct config *config, const struct command *cmd);
