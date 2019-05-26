#include "cmdalias.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define reset_result(r)                                                        \
  do {                                                                         \
    r->argc = 0;                                                               \
  } while (0)

#define add_str_to_result(r, s)                                                \
  do {                                                                         \
    r->argv[r->argc++] = s;                                                    \
  } while (0)

#define add_str_list_to_result(r, l)                                           \
  do {                                                                         \
    string_list *str_list = l;                                                 \
    while (str_list) {                                                         \
      r->argv[r->argc++] = str_list->str;                                      \
      str_list = str_list->next;                                               \
    }                                                                          \
  } while (0)

int alias_execute_recursive(int argc, char **argv, alias_list *aliases,
                            global_alias_list *globals,
                            alias_execute_result *result) {
  int r = 1, is_cmd = 0;
  if (argc) {
    alias *a = get_alias(aliases, argv[0]);
    if (a) {
      if (a->global_alias_list) {
        globals = global_alias_list_prepend(globals, a->global_alias_list);
      }
      if (a->is_cmd) {
        reset_result(result);
        is_cmd = 1;
      }
      add_str_list_to_result(result, a->substitutes);
      aliases = a->sub_alias_list;
    } else if ((a = get_global_alias(globals, argv[0]))) {
      add_str_list_to_result(result, a->substitutes);
      aliases = NULL;
    } else {
      add_str_to_result(result, argv[0]);
      aliases = NULL;
    }

    r &= alias_execute_recursive(argc - 1, argv + 1, aliases, globals, result);

    if (a && r) {
      add_str_list_to_result(result, a->substitutes_after);
    }
  }

  return r && !is_cmd;
}

alias_execute_result *alias_execute(command_list *commands, int argc,
                                    char **argv) {

  alias_execute_result *result =
      (alias_execute_result *)malloc(sizeof(alias_execute_result));
  result->argc = 0;

  alias_list *aliases = NULL;
  global_alias_list *globals = NULL;
  command *cmd = get_cmd(commands, argv[0]);

  if (cmd) {

    aliases = cmd->alias_list;
    if (cmd->global_alias_list) {
      globals = global_alias_list_prepend(globals, cmd->global_alias_list);
    }

    add_str_to_result(result, cmd->name);

    add_str_list_to_result(result, cmd->before_args);
    if (alias_execute_recursive(argc - 1, argv + 1, aliases, globals, result)) {
      add_str_list_to_result(result, cmd->after_args);
    }
  } else {
    add_str_to_result(result, argv[0]);
  }

  add_str_to_result(result, NULL);
  global_alias_list_delete(globals);

  return result;
  /*
  #if CMDALIAS_DEBUG
    debug_msg("Executing:\n");
    for (int i = 0; result->argv[i] != NULL; i++) {
      debug_msg("\t[%d] %s\n", i, result->argv[i]);
    }
    return 1;
  #endif

    execvp(result->argv[0], result->argv);

    fprintf(stderr, "cmdalias: %s: %s\n", result->argv[0], strerror(errno));

    return -1;
  */
}
