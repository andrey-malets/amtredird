#include "config.h"

#include <stdio.h>

int main(int ac, char *av[]) {
  struct config *config = parse_config("amtredird.ini");
  if (config) {
    puts(config->amt_ini_filename);
  }

  free_config(config);
}
