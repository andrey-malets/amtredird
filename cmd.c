#include "amt.h"
#include "cmd.h"
#include "config.h"
#include "macro.h"

#include "cmp/cmp.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PROTO_VERSION 1

enum {
  CMD_LIST,
  CMD_START,
  CMD_STOP,
  CMD_LAST = CMD_STOP
} cmd_type;


enum {
  RES_OK,
  RES_NO_SUCH_CLIENT,
  RES_FAILED,
  RES_NOT_UNDERSTOOD
} result_code;

static bool read_from_socket(cmp_ctx_t *ctx, void *data, size_t size) {
  const int *socket = ctx->buf;
  ssize_t rem = size, received = 0;
  assert(rem);
  do {
    CHECK(SYSCALL(received = recv(
              *socket, (char *)data + (size - rem), rem, MSG_WAITALL)),
          perror("recv() failed"), return false);
    rem -= received;
  } while (rem && received);

  return rem == 0;
}

static size_t write_to_socket(cmp_ctx_t *ctx, const void *data, size_t size) {
  const int *socket = ctx->buf;
  ssize_t rem = size, sent = 0;
  assert(rem);
  do {
    CHECK(SYSCALL(sent = send(
              *socket, (char *)data + (size - rem), rem, MSG_NOSIGNAL)),
          perror("send() failed"), return 0);
    rem -= sent;
  } while (rem && sent);
  return rem == 0 ? size : 0;
}

int read_cmd(int socket, struct command *cmd) {
  cmp_ctx_t ctx;
  cmp_init(&ctx, &socket, read_from_socket, NULL);

  uint32_t size;
  if (!cmp_read_array(&ctx, &size) || size < 2)
    return 0;

  uint8_t version;
  if (!cmp_read_uchar(&ctx, &version) || version != PROTO_VERSION)
    return 0;

  if (!cmp_read_uchar(&ctx, &cmd->type) || cmd->type > CMD_LAST)
    return 0;

  switch (cmd->type) {
  case CMD_START:
  case CMD_STOP: {
    uint32_t argsize = sizeof(cmd->arg);
    if (size != 3 || !cmp_read_str(&ctx, cmd->arg, &argsize))
      return 0;
    break;
  }
  default:
    if (size != 2)
      return 0;
  }

  return 1;
}

int write_result(int socket, const struct result *result) {
  cmp_ctx_t ctx;
  cmp_init(&ctx, &socket, NULL, write_to_socket);

  if (!cmp_write_array(&ctx, 2 + result->num_clients) ||
      !cmp_write_u8(&ctx, PROTO_VERSION) || !cmp_write_u8(&ctx, result->code))
    return 0;

  for (uint32_t i = 0; i != result->num_clients; ++i) {
    const char *host = result->clients[i].host;
    if (!cmp_write_str(&ctx, host, strlen(host)))
      return 0;
  }

  return 1;
}

struct result execute(const struct config *config, const struct command *cmd) {
  struct result res;
  memset(&res, 0, sizeof(res));
  res.code = RES_NOT_UNDERSTOOD;
  const struct client *client = NULL;
  int rv;
  switch (cmd->type) {
  case CMD_LIST:
    assert(config->num_clients <= UINT32_MAX);
    res.code = RES_OK;
    res.num_clients = config->num_clients;
    res.clients = config->clients;
    break;
  case CMD_START:
  case CMD_STOP:
    if ((client = find_client(config, cmd->arg))) {
      rv = cmd->type == CMD_START ?
          start_client(config, client) : stop_client(client);
      res.code = (rv == 1) ? RES_OK : RES_FAILED;
    } else {
      res.code = RES_NO_SUCH_CLIENT;
    }
    break;
  }

  return res;
}
