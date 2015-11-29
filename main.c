#include "amt.h"
#include "config.h"
#include "server.h"

#include <stdio.h>

int main(int ac, char *av[]) {
  int rv = 0;

  struct config *config = parse_config("amtredird.ini");
  if (config && validate_config(config)) {
    if (init_amt(config)) {
      rv = (run_server(config) == 0);
      teardown_amt(config);
    }
  } else {
    rv = 1;
  }
  free_config(config);
  return rv;
}
