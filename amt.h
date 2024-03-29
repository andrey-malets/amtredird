#pragma once

#include "config.h"

void display_error(const char *host, const char *cmd, IMRResult res);

int init_amt(struct config *config);
void teardown_amt(const struct config *config);

int start_client(const struct config *config, const struct client *client);
int stop_client(const struct client *client);
