#pragma once

#include <stdint.h>
#include <stdlib.h>

struct client {
  const char *host;
  const char *user;
  const char *passwd;
  const char *filename;
};

struct config {
  const char *amt_ini_filename;
  const char *default_user;
  const char *default_passwd;

  size_t num_clients;

  char *char_storage;

  // sorted by host
  struct client clients[];
};

struct config *parse_config(const char *filename);
void free_config(struct config *config);
