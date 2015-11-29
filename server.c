#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "config.h"
#include "macro.h"
#include "server.h"

int run_server(const struct config *config) {
  int server_sock;
  int rv = 1;
  CHECK(SYSCALL(server_sock = socket(AF_UNIX, SOCK_STREAM, 0)),
        PERROR1("socket() failed for", "server"),
        GOTO_WITH(exit, rv, 0));

  struct sockaddr_un server_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  // TODO: validate length while parsing config.
  assert(config->socket);
  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, config->socket,
          sizeof(server_addr.sun_path) - 1);

  CHECK(SYSCALL(bind(server_sock, (struct sockaddr *) &server_addr,
                     sizeof(server_addr))),
        PERROR1("bind() failed with", config->socket),
        GOTO_WITH(close, rv, 0));

  CHECK(SYSCALL(listen(server_sock, 1)),
        PERROR1("listen() failed for", config->socket),
        GOTO_WITH(close, rv, 0));

  // TODO: accept, read cmd, execute, write reply.

close:
  CHECK(SYSCALL(close(server_sock)),
        PERROR1("close() for", "server socket"), ;);
exit:
  return rv;
}
