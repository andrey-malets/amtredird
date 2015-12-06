#pragma once

#include "amt-redir-libs/include/IMRSDK.h"

#include <stdint.h>
#include <stdlib.h>

struct client {
  const char *host;
  const char *user;
  const char *passwd;
  const char *filename;

  ClientID id;
};

struct config {
  const char *amt_ini_filename;

  const char *socket;

  const char *default_user;
  const char *default_passwd;

  size_t num_clients;

  char *char_storage;

  // sorted by host
  struct client clients[];
};

struct config *parse_config(const char *filename);
int validate_config(const struct config *config);

const char *get_username(const struct config *config,
                         const struct client *client);
const char *get_passwd(const struct config *config,
                       const struct client *client);

void free_config(struct config *config);

struct client *find_client(const struct config *config, const char *host);
