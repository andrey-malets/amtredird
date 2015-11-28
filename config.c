#include "config.h"
#include "inih/ini.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SECTION  "amtredird"
#define AMT_INI_FILENAME "amt_ini_filename"
#define DEFAULT_USER     "default_user"
#define DEFAULT_PASSWD   "default_passwd"

#define HOST     "host"
#define USER     "user"
#define PASSWD   "passwd"
#define FILENAME "filename"

struct parse_state {
  size_t num_clients;
  size_t char_storage_size;

  struct config *config;
  char *char_storage_ptr;

  size_t current_client;
  char current_section[INI_MAX_LINE];
};

static int is_default_section(const char *section) {
  return strncmp(section, DEFAULT_SECTION, INI_MAX_LINE) == 0;
}

static int size_handler(void *user, const char *section,
                        const char *name, const char *value) {
  struct parse_state *state = user;
  if (strcmp(state->current_section, section)) {
    strcpy(state->current_section, section);
    if (!is_default_section(section))
      ++state->num_clients;
  }

  state->char_storage_size += strlen(value) + 1;
  return 1;
}

static int config_handler(void *user, const char *section,
                          const char *name, const char *value) {
  struct parse_state *state = user;
  if (strcmp(state->current_section, section)) {
    strcpy(state->current_section, section);
    if (!is_default_section(section))
      ++state->current_client;
  }

#define HANDLE_PARAM(param, output) \
  do { \
    if (strcmp(name, param) == 0) { \
      const size_t len = strlen(value) + 1; \
      assert(state->char_storage_ptr + len <= \
             state->config->char_storage + state->char_storage_size); \
      strcpy(state->char_storage_ptr, value); \
      state->config->output = state->char_storage_ptr; \
      state->char_storage_ptr += len; \
      return 1; \
    } \
  } while (0)

  if (is_default_section(section)) {
    HANDLE_PARAM(AMT_INI_FILENAME, amt_ini_filename);
    HANDLE_PARAM(DEFAULT_USER, default_user);
    HANDLE_PARAM(DEFAULT_PASSWD, default_passwd);
  } else {
#define CLIENT clients[state->current_client-1]
    HANDLE_PARAM(HOST, CLIENT.host);
    HANDLE_PARAM(USER, CLIENT.user);
    HANDLE_PARAM(PASSWD, CLIENT.passwd);
    HANDLE_PARAM(FILENAME, CLIENT.filename);
#undef CLIENT
  }

#undef HANDLE_PARAM

  return 0;
}

static int get_sizes(const char *filename, struct parse_state *output) {
  memset(output->current_section, 0, INI_MAX_LINE);
  assert(output);

  return ini_parse(filename, size_handler, output) == 0;
}

static int compare_hostnames(const void *lhs, const void *rhs) {
  const struct client *left = lhs, *right = rhs;
  return strcmp(left->host, right->host);
}

struct config *parse_config(const char *filename) {
  struct parse_state state;
  memset(&state, 0, sizeof(struct parse_state));

  if (!get_sizes(filename, &state))
    return NULL;

  const size_t config_size = sizeof(struct config) +
                             state.num_clients * sizeof(struct client);
  if (!(state.config = malloc(config_size)))
    goto free_rv;
  memset(state.config, 0, config_size);
  state.config->num_clients = state.num_clients;

  if (!(state.config->char_storage = malloc(state.char_storage_size)))
    goto free_storage;
  state.char_storage_ptr = state.config->char_storage;

  memset(state.current_section, 0, sizeof(state.current_section));
  if (ini_parse(filename, config_handler, &state) != 0)
    goto free_storage;

  qsort(state.config->clients, state.config->num_clients,
        sizeof(struct client), compare_hostnames);

  return state.config;

free_storage:
  free(state.config->char_storage);

free_rv:
  free(state.config);
  return NULL;
}

void free_config(struct config *config) {
  if (config)
    free(config->char_storage);
  free(config);
}
