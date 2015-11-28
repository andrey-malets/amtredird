#include "amt.h"
#include "amt-redir-libs/include/IMRSDK.h"

#include <stdio.h>

int init_amt(struct config *config) {
  // Unfortunately, they do not seem to use C constructs properly.
  char *filename = (char *)config->amt_ini_filename;
  IMRResult res = IMR_Init(NULL, filename);

  // TODO: get the error message from the bullshit interface.
  if (res != IMR_RES_OK)
    goto close;

  size_t i;
  for (i = 0; i != config->num_clients; ++i) {
    struct client client = config->clients[i];
    res = IMR_AddClient(CLI_TCP, (char *)client.host, NULL, &client.id);
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
  for (size_t i = config->num_clients; i-- != 0;)
    IMR_RemoveClient(config->clients[i].id);
  IMR_Close();
}
