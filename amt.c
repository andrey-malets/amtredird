#include "amt.h"
#include "amt-redir-libs/include/IMRSDK.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define EMPTY_DEV "/dev/zero"

#define ERROR_MAX_CHARS 200

#define GOTO_WITH(label, retval, value) \
  do { \
    (retval) = (value); \
    goto label; \
  } while (0)

#define IMR_CHECK(call, act) \
  do { \
    IMRResult res = call; \
    if (res != IMR_RES_OK) {\
      display_error(res); \
      act; \
    } \
  } while (0)

void display_error(IMRResult res) {
  if (res != IMR_RES_OK) {
    char error[ERROR_MAX_CHARS];
    int len;
    switch (IMR_GetErrorStringLen(res, &len)) {
    case IMR_RES_OK:
      break;
    case IMR_RES_UNKNOWN:
      fprintf(stderr, "Error #%d is unknown to AMT\n", res);
      return;
    default:
      assert(0);
      break;
    }

    if (len > sizeof(error)) {
      fprintf(stderr, "Too long AMT error description for #%d\n", res);
    } else {
      assert(IMR_GetErrorString(res, error) == IMR_RES_OK);
      fprintf(stderr, "AMT error #%d: %s\n", res, error);
    }
  }
}

int init_amt(struct config *config) {
  assert(config);
  assert(config->amt_ini_filename);

  IMR_CHECK(IMR_Init(NULL, (char *)config->amt_ini_filename), goto close);

  size_t i;
  for (i = 0; i != config->num_clients; ++i) {
    struct client client = config->clients[i];
    assert(client.host);
    IMR_CHECK(IMR_AddClient(CLI_TCP, (char *)client.host, NULL, &client.id),
              goto remove_clients);
  }

  return 1;

remove_clients:
  for (size_t j = i; j-- != 0;)
    IMR_CHECK(IMR_RemoveClient(config->clients[j].id), ;);

close:
  IMR_Close();
  return 0;
}

void teardown_amt(const struct config *config) {
  assert(config);
  for (size_t i = config->num_clients; i-- != 0;)
    IMR_CHECK(IMR_RemoveClient(config->clients[i].id), ;);
  IMR_CHECK(IMR_Close(), ;);
}

int start_client(const struct config *config, const struct client *client) {
  assert(config);
  assert(client);
  assert(client->filename);

  TCPSessionParams params;
  strncpy(params.user_name, get_username(config, client), MAX_NAME_LEN);
  strncpy(params.user_pswd, get_passwd(config, client), MAX_PSWD_LEN);

  IMR_CHECK(
      IMR_IDEROpenTCPSession(client->id, &params, NULL,
                             (char *)client->filename, (char *)EMPTY_DEV),
      goto ret);

  IDERDeviceCmd cmd = {.pri_op = IDER_ENABLE, .pri_timing = IDER_SET_ONRESET};
  IDERDeviceResult result;
  IMR_CHECK(IMR_IDERSetDeviceState(client->id, &cmd, &result), goto free);

  if (result.pri_res != IDER_DONE) {
    fputs("IDERDeviceResult is not IDER_DONE\n", stderr);
    goto free;
  }

  return 1;

free:
  IMR_CHECK(IMR_IDERCloseSession(client->id), ;);
ret:
  return 0;
}

int stop_client(const struct client *client) {
  assert(client);
  int rv = 1;

  IDERDeviceCmd cmd = {.pri_op = IDER_DISABLE,
                       .pri_timing = IDER_SET_IMMEDIATELY};
  IDERDeviceResult result;
  IMR_CHECK(IMR_IDERSetDeviceState(client->id, &cmd, &result),
            GOTO_WITH(free, rv, 0));

  if (result.pri_res != IDER_DONE) {
    fputs("warning: IDERDeviceResult is not IDER_DONE\n", stderr);
    rv = 0;
  }

free:
  IMR_CHECK(IMR_IDERCloseSession(client->id), GOTO_WITH(ret, rv, 0));
ret:
  return rv;
}
