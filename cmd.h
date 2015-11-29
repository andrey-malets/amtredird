#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cmp/cmp.h"

#define MAX_ARG_SIZE 128

enum {
  CMD_LIST,
  CMD_ADD,
  CMD_DELETE,
  CMD_LAST = CMD_DELETE
} cmd_type;

struct command {
  uint8_t type;
  char arg[MAX_ARG_SIZE];
};

enum {
  RES_OK,
  RES_FAILED
} result_code;

struct result {
  uint8_t code;
};

int read_cmd(void *data, cmp_reader reader, struct command *cmd);
int write_result(void *data, cmp_writer writer, const struct result *result);
