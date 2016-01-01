#include <sys/signalfd.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "amt.h"
#include "config.h"
#include "macro.h"
#include "server.h"

int setup_signalfd() {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGPIPE);
  sigaddset(&mask, SIGTERM);
  int rv = -1;
  CHECK(SYSCALL(sigprocmask(SIG_BLOCK, &mask, NULL)),
        perror("sigprocmask(SIG_BLOCK, ...) failed"), goto ret);
  sigdelset(&mask, SIGPIPE);
  CHECK(SYSCALL(rv = signalfd(-1, &mask, 0)),
        perror("signalfd() failed"), goto ret);
ret:
  return rv;
}

int main(int ac, char *av[]) {
  int rv = 0;

  int sfd = setup_signalfd();

  struct config *config = parse_config("amtredird.ini");
  if (config && validate_config(config)) {
    if (init_amt(config)) {
      if (sfd != -1) {
        rv = (run_server(config, sfd) == 0);
      } else {
        rv = 1;
      }
      teardown_amt(config);
    }
  } else {
    rv = 1;
  }
  close(sfd);
  free_config(config);
  return rv;
}
