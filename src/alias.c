#include "cmdalias.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static command *get_cmd(command_list *list, const char *cmd) {
  while (list) {
    string_list *name_aliases = NULL;

    if (0 == strcmp(cmd, list->command->name)) {
      return list->command;
    }

    name_aliases = list->command->name_aliases;
    while (name_aliases) {
      if (0 == strcmp(cmd, name_aliases->str)) {
        return list->command;
      }
      name_aliases = name_aliases->next;
    }

    list = list->next;
  }

  return NULL;
}

static alias *get_alias(alias_list *list, const char *name) {
  while (list) {
    string_list *name_list = list->alias->names;
    while (name_list) {
      if (0 == strcmp(name, name_list->str)) {
        return list->alias;
      }
      name_list = name_list->next;
    }
    list = list->next;
  }

  return NULL;
}

static alias *get_global_alias(global_alias_list *globals, const char *name) {
  alias *result = NULL;
  while (globals) {
    if ((result = get_alias(globals->alias_list, name))) {
      return result;
    }

    globals = globals->next;
  }

  return NULL;
}

struct result_t {
  int argc;
  char *argv[100];
};

void alias_execute_recursive(int argc, char **argv, alias_list *aliases,
                             global_alias_list *globals,
                             struct result_t *result) {
  if (argc) {
    string_list *str_list;
    alias *a = get_alias(aliases, argv[0]);
    if (a) {
      str_list = a->substitutes;

      if (a->global_alias_list) {
        globals = global_alias_list_append(globals, a->global_alias_list);
      }

      if (a->is_cmd) {
        result->argc = 0;
      }

      while (str_list) {
        result->argv[result->argc++] = str_list->str;
        str_list = str_list->next;
      }

      aliases = a->sub_alias_list;
    } else if ((a = get_global_alias(globals, argv[0]))) {
      str_list = a->substitutes;
      while (str_list) {
        result->argv[result->argc++] = str_list->str;
        str_list = str_list->next;
      }
      aliases = NULL;
    } else {
      result->argv[result->argc++] = argv[0];
      aliases = NULL;
    }

    alias_execute_recursive(argc - 1, argv + 1, aliases, globals, result);

    if (a) {
      str_list = a->substitutes_after;
      while (str_list) {
        result->argv[result->argc++] = str_list->str;
        str_list = str_list->next;
      }
    }
  }
}

int alias_execute(command_list *commands, int argc, char **argv) {

  struct result_t result = {0};

  alias_list *aliases = NULL;
  global_alias_list *globals = NULL;

  command *cmd = get_cmd(commands, argv[0]);

  if (cmd) {
    string_list *str_list = cmd->before_args;
    aliases = cmd->alias_list;
    if (cmd->global_alias_list) {
      globals = global_alias_list_append(globals, cmd->global_alias_list);
    }

    result.argv[result.argc++] = cmd->name;
    while (str_list) {
      result.argv[result.argc++] = str_list->str;
      str_list = str_list->next;
    }
  } else {
    result.argv[result.argc++] = argv[0];
  }

  alias_execute_recursive(argc - 1, argv + 1, aliases, globals, &result);

  if (cmd) {
    string_list *str_list = cmd->after_args;
    while (str_list) {
      result.argv[result.argc++] = str_list->str;
      str_list = str_list->next;
    }
  }

  result.argv[result.argc++] = NULL;
  global_alias_list_delete(globals);

#if CMDALIAS_DEBUG
  debug_msg("Executing:\n");
  for (int i = 0; result.argv[i] != NULL; i++) {
    debug_msg("\t[%d] %s\n", i, result.argv[i]);
  }
  return 1;
#endif

  return execvp(result.argv[0], result.argv);
}
