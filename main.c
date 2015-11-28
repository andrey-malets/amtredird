#include "amt.h"
#include "config.h"

#include <stdio.h>

int main(int ac, char *av[]) {
  int rv = 1;

  struct config *config = parse_config("amtredird.ini");
  if (config) {
    if (init_amt(config)) {
      rv = 0;
      teardown_amt(config);
    }
  }
  free_config(config);
  return rv;
}
