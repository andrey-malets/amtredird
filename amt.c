#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "amt.h"
#include "amt-redir-libs/include/IMRSDK.h"
#include "macro.h"

#define EMPTY_DEV "/dev/zero"

#define ERROR_MAX_CHARS 200

static const IDERTout IDER_TIMEOUTS = {
  .rx_timeout = 10000,
  .hb_timeout = 2000,
  .tx_timeout = 0
};

void display_error(const char *host, const char *cmd, IMRResult res) {
  assert(cmd);
  if (res != IMR_RES_OK) {
    if (host) {
      fprintf(stderr, "%s for %s failed: ", cmd, host);
    } else {
      fprintf(stderr, "%s failed: ", cmd);
    }

    char error[ERROR_MAX_CHARS];
    int len;
    switch (IMR_GetErrorStringLen(res, &len)) {
    case IMR_RES_OK:
      break;
    case IMR_RES_UNKNOWN:
      fprintf(stderr, "error #%d is unknown to AMT\n", res);
      return;
    default:
      assert(0);
      break;
    }

    if (len > sizeof(error)) {
      fprintf(stderr, "too long AMT error description for #%d\n", res);
    } else {
      assert(IMR_GetErrorString(res, error) == IMR_RES_OK);
      fprintf(stderr, "%s (%d)\n", error, res);
    }
  }
}

int init_amt(struct config *config) {
  assert(config);
  assert(config->amt_ini_filename);

  IMR_CHECK(goto close, NULL, IMR_Init, NULL, (char *)config->amt_ini_filename);

  size_t i;
  for (i = 0; i != config->num_clients; ++i) {
    struct client *client = &config->clients[i];
    assert(client->host);
    IMR_CHECK(goto remove_clients, client->host,
              IMR_AddClient, CLI_TCP, (char *)client->host, NULL, &client->id);
  }

  return 1;

remove_clients:
  for (size_t j = i; j-- != 0;)
    IMR_CHECK(;, config->clients[j].host,
              IMR_RemoveClient, config->clients[j].id);
close:
  IMR_CHECK(;, NULL, IMR_Close);
  return 0;
}

void teardown_amt(const struct config *config) {
  assert(config);
  for (size_t i = config->num_clients; i-- != 0;)
    IMR_CHECK(;, config->clients[i].host,
              IMR_RemoveClient, config->clients[i].id);
  IMR_CHECK(;, NULL, IMR_Close);
}

int start_client(const struct config *config, const struct client *client) {
  assert(config);
  assert(client);
  assert(client->filename);

  TCPSessionParams params;
  strncpy(params.user_name, get_username(config, client), MAX_NAME_LEN);
  strncpy(params.user_pswd, get_passwd(config, client), MAX_PSWD_LEN);

  IMR_RETRY_ON_TIMEOUT(5, goto ret, client->host, IMR_IDEROpenTCPSession,
                       client->id, &params, (IDERTout *)&IDER_TIMEOUTS,
                       (char *)client->filename, (char *)EMPTY_DEV);

  IDERDeviceCmd cmd = {.pri_op = IDER_ENABLE, .pri_timing = IDER_SET_ONRESET};
  IDERDeviceResult result;
  IMR_RETRY_ON_TIMEOUT(5, goto free, client->host,
                       IMR_IDERSetDeviceState, client->id, &cmd, &result);

  if (result.pri_res != IDER_DONE) {
    fputs("IDERDeviceResult is not IDER_DONE\n", stderr);
    goto free;
  }

  return 1;

free:
  IMR_CHECK(;, client->host, IMR_IDERCloseSession, client->id);
ret:
  return 0;
}

int stop_client(const struct client *client) {
  assert(client);
  int rv = 1;

  IDERDeviceCmd cmd = {.pri_op = IDER_DISABLE, .pri_timing = IDER_SET_ONRESET};
  IDERDeviceResult result;
  IMR_RETRY_ON_TIMEOUT(5, GOTO_WITH(free, rv, 0), client->host,
                       IMR_IDERSetDeviceState, client->id, &cmd, &result);

  if (result.pri_res != IDER_DONE) {
    fputs("warning: IDERDeviceResult is not IDER_DONE\n", stderr);
    rv = 0;
  }

free:
  IMR_RETRY_ON_TIMEOUT(5, GOTO_WITH(ret, rv, 0),
                       client->host, IMR_IDERCloseSession, client->id);
ret:
  return rv;
}
