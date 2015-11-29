#include "cmd.h"

#define PROTO_VERSION 1

int read_cmd(void *data, cmp_reader reader, struct command *cmd) {
  cmp_ctx_t ctx;
  cmp_init(&ctx, data, reader, NULL);

  uint8_t version;
  if (!cmp_read_u8(&ctx, &version) || version != PROTO_VERSION)
    return 0;

  if (!cmp_read_u8(&ctx, &cmd->type) || cmd->type > CMD_LAST)
    return 0;

  uint32_t size = sizeof(cmd->arg);
  if (!cmp_read_str(&ctx, cmd->arg, &size))
    return 0;

  return 1;
}

int write_result(void *data, cmp_writer writer, const struct result *result) {
  cmp_ctx_t ctx;
  cmp_init(&ctx, data, NULL, writer);

  return cmp_write_u8(&ctx, PROTO_VERSION) && cmp_write_u8(&ctx, result->code);
}
