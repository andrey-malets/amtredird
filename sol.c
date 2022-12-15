#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "amt.h"
#include "config.h"
#include "macro.h"


static const SOLTout SOL_TIMEOUTS = {
  .tx_over_timeout = 0,
  .tx_buf_timeout = 100,
  .hb_interval = 5000,
  .fifo_rx_flush_timeout = 100,
  .rx_timeout = 10000
};


int disable_raw_mode(struct termios *orig) {
  return tcsetattr(STDIN_FILENO, TCSAFLUSH, orig);
}


int enable_raw_mode(struct termios *orig) {
  if (tcgetattr(STDIN_FILENO, orig) == -1)
		return 0;

  struct termios raw = *orig;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		return 0;

  return 1;
}


enum escape_state { ES_OTHER, ES_ENTER, ES_AT };

enum escape_action { ACT_CONTINUE, ACT_HOLD, ACT_END };

int init_escape() {
  return ES_OTHER;
}

int process_escape(int *state, char input) {
  switch (*state) {
  case ES_OTHER:
    if (input == '\r') {
      *state = ES_ENTER;
    }
    return ACT_CONTINUE;
  case ES_ENTER:
    if (input == '@') {
      *state = ES_AT;
      return ACT_HOLD;
    } else {
      *state = ES_OTHER;
      return ACT_CONTINUE;
    }
  case ES_AT:
    if (input == '.') {
      return ACT_END;
    } else {
      *state = ES_OTHER;
      return ACT_CONTINUE;
    }
  default:
    assert(0);
  }
}

int run_sol(const struct config *config, const struct client *client) {
  assert(config);
  assert(client);

  TCPSessionParams params;
  strncpy(params.user_name, get_username(config, client), MAX_NAME_LEN);
  strncpy(params.user_pswd, get_passwd(config, client), MAX_PSWD_LEN);

  IMR_CHECK(return 1, client->host, IMR_SOLOpenTCPSession,
            client->id, &params, (SOLTout *)&SOL_TIMEOUTS, NULL);

  struct termios orig;
  enable_raw_mode(&orig);

  char input = '\0';
  unsigned char output[64 * 1024];
  int es = init_escape();
  int stop = 0;

  while (!stop) {
    ssize_t read_rv = read(STDIN_FILENO, &input, 1);
    if (read_rv == -1) {
      break;
    } else if (read_rv == 1) {
      switch (process_escape(&es, input)) {
      case ACT_CONTINUE:
        IMR_CHECK(goto teardown, client->host, IMR_SOLSendText,
                  client->id, (unsigned char *)&input, 1);
        break;
      case ACT_HOLD:
        break;
      case ACT_END:
        stop = 1;
        break;
      }
    }
    size_t len = sizeof(output);
    IMR_CHECK(goto teardown, client->host, IMR_SOLReceiveText,
              client->id, &output[0], (int *)&len);
    write(STDOUT_FILENO, output, len);
  }

teardown:
  disable_raw_mode(&orig);
  IMR_CHECK(return 1, client->host, IMR_SOLCloseSession, client->id);
  return 0;
}


int main(int ac, char *av[]) {
  int rv = 0;

  if (ac != 2) {
    return 2;
  }

  struct config *config = parse_config("amtredird.ini");
  if (config && validate_config(config)) {
    if (init_amt(config)) {
      const char *host = av[1];
      const struct client *client = NULL;
      if ((client = find_client(config, host))) {
        rv = run_sol(config, client);
      } else {
        rv = 3;
      }

      teardown_amt(config);
    }
  } else {
    rv = 1;
  }
  free_config(config);
  return rv;
}
