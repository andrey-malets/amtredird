#include "amt.h"
#include "amt-redir-libs/include/IMRSDK.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define EMPTY_DEV "/dev/zero"

#define ERROR_MAX_CHARS 200

void handle_error(IMRResult res) {
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

  // Unfortunately, they do not seem to use C constructs properly.
  char *filename = (char *)config->amt_ini_filename;
  IMRResult res = IMR_Init(NULL, filename);

  // TODO: get the error message from the bullshit interface.
  if (res != IMR_RES_OK)
    goto close;

  size_t i;
  for (i = 0; i != config->num_clients; ++i) {
    struct client client = config->clients[i];
    assert(client.host);
    res = IMR_AddClient(CLI_TCP, (char *)client.host, NULL, &client.id);
    handle_error(res);
    if (res != IMR_RES_OK)
      goto remove_clients;
  }

  return 1;

remove_clients:
  for (size_t j = i; j-- != 0;)
    IMR_RemoveClient(config->clients[j].id);

close:
  IMR_Close();
  return 0;
}

void teardown_amt(const struct config *config) {
  assert(config);
  for (size_t i = config->num_clients; i-- != 0;)
    IMR_RemoveClient(config->clients[i].id);
  IMR_Close();
}

int start_client(const struct config *config, const struct client *client) {
  assert(config);
  assert(client);
  assert(client->filename);

  TCPSessionParams params;
  strncpy(params.user_name, get_username(config, client), MAX_NAME_LEN);
  strncpy(params.user_pswd, get_passwd(config, client), MAX_PSWD_LEN);

  IMRResult res = IMR_IDEROpenTCPSession(
      client->id, &params, NULL, (char *)client->filename, (char *)EMPTY_DEV);
  handle_error(res);

  return res == IMR_RES_OK;
}

int stop_client(const struct client *client) {
  assert(client);
  return IMR_IDERCloseSession(client->id) == IMR_RES_OK;
}
