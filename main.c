#include "amt.h"
#include "config.h"

#include <stdio.h>

int main(int ac, char *av[]) {
  int rv = 0;

  struct config *config = parse_config("amtredird.ini");
  if (config && validate_config(config)) {
    if (init_amt(config)) {
      for (size_t i = 0; i != config->num_clients; ++i) {
        const struct client *client =
            find_client(config, config->clients[i].host);
        if (!start_client(config, client))
          rv = 1;
      }

      if (rv == 0)
        fgetc(stdin);

      for (size_t i = 0; i != config->num_clients; ++i) {
        if (!stop_client(&config->clients[i]))
          rv = 1;
      }

      teardown_amt(config);
    }
  } else {
    rv = 1;
  }
  free_config(config);
  return rv;
}
