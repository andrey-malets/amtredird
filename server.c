#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "cmd.h"
#include "config.h"
#include "macro.h"
#include "server.h"

int max(int left, int right) {
  return left > right ? left : right;
}

int set_timeouts(int sock, const struct timeval timeout) {
  int rv = -1;
  CHECK(SYSCALL(rv = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                                &timeout, sizeof(timeout))),
        perror("setsockopt(SO_RECVTIMEO) failed"), goto ret);

  CHECK(SYSCALL(rv = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,
                                &timeout, sizeof(timeout))),
        perror("setsockopt(SO_SNDTIMEO) failed"), goto ret);
ret:
  return rv;
}

int run_server(const struct config *config, int sfd) {
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

  CHECK(SYSCALL(chmod(config->socket, 0666)),
        PERROR1("chmod() failed with", config->socket),
        GOTO_WITH(close, rv, 0));

  CHECK(SYSCALL(listen(server_sock, 1)),
        PERROR1("listen() failed for", config->socket),
        GOTO_WITH(close, rv, 0));

  for (;;) {
    int srv = -1;
    const int maxfd = 1 + max(sfd, server_sock);

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sfd, &rfds);
    FD_SET(server_sock, &rfds);

    CHECK(SYSCALL(srv = select(maxfd, &rfds, NULL, NULL, NULL)),
          perror("select() failed"),
          GOTO_WITH(close, rv, 0));

    // TODO: check for particular signals.
    if (FD_ISSET(sfd, &rfds))
      break;

    if (FD_ISSET(server_sock, &rfds)) {
      int client_sock;
      CHECK(SYSCALL(client_sock = accept(server_sock, NULL, NULL)),
            perror("accept() failed"), goto finish_client);

      struct timeval timeout;
      memset(&timeout, 0, sizeof(timeout));
      timeout.tv_sec = 10;

      CHECK(SYSCALL(set_timeouts(client_sock, timeout)), ;, goto finish_client);

      struct command cmd;
      CHECK(read_cmd(client_sock, &cmd), ;, goto finish_client);

      const struct result result = execute(config, &cmd);

      CHECK(write_result(client_sock, &result), ;, ;);

finish_client:
      CHECK(SYSCALL(close(client_sock)), perror("close() failed"), ;);
    }
  }

close:
  CHECK(SYSCALL(close(server_sock)),
        PERROR1("close() failed for", "server socket"),
        GOTO_WITH(exit, rv, 0));
  CHECK(SYSCALL(unlink(config->socket)),
        PERROR1("ulink() failed for", config->socket),
        GOTO_WITH(exit, rv, 0));
exit:
  return rv;
}
