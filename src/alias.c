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

int alias_execute(command_list *commands, int argc, char **argv) {
  int args_c = 0;
  char *args[100];
  int i;
  alias *a = NULL;
  alias_list *aliases = NULL;
  global_alias_list *globals = NULL;

  command *cmd = get_cmd(commands, argv[0]);

  if (cmd) {
    string_list *str_list = cmd->args;
    aliases = cmd->alias_list;
    if (cmd->global_alias_list) {
      globals = global_alias_list_append(globals, cmd->global_alias_list);
    }

    args[args_c++] = cmd->name;
    while (str_list) {
      args[args_c++] = str_list->str;
      str_list = str_list->next;
    }
  } else {
    args[args_c++] = argv[0];
  }

  for (i = 1; i < argc; i++) {
    a = get_alias(aliases, argv[i]);
    if (a) {
      string_list *str_list = a->substitutes;

      if (a->global_alias_list) {
        globals = global_alias_list_append(globals, a->global_alias_list);
      }

      if (a->is_cmd) {
        args_c = 0;
      }

      while (str_list) {
        args[args_c++] = str_list->str;
        str_list = str_list->next;
      }

      aliases = a->sub_alias_list;
    } else if ((a = get_global_alias(globals, argv[i]))) {
      string_list *str_list = a->substitutes;
      while (str_list) {
        args[args_c++] = str_list->str;
        str_list = str_list->next;
      }
      aliases = NULL;
    } else {
      args[args_c++] = argv[i];
      aliases = NULL;
    }
  }

  args[args_c++] = NULL;
  global_alias_list_delete(globals);

#if CMDALIAS_DEBUG
  debug_msg("Executing:\n");
  for (i = 0; args[i] != NULL; i++) {
    debug_msg("\t[%d] %s\n", i, args[i]);
  }
  return 1;
#endif

  return execvp(args[0], args);
}
