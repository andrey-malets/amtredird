#include "config.h"

#include <stdio.h>

int main(int ac, char *av[]) {
  struct config *config = parse_config("amtredird.ini");
  free_config(config);
}
