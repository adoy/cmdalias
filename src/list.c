#include "cmdalias.h"
#include <stdlib.h>

static void string_list_free_all(string_list *list) {
  string_list *item;
  while ((item = list)) {
    list = item->next;
    free(item->str);
    free(item);
  }
}

static void free_alias(alias *a) {
  string_list_free_all(a->names);
  string_list_free_all(a->substitutes);
  string_list_free_all(a->substitutes_after);
  alias_list_free_all(a->sub_alias_list);
  alias_list_free_all(a->global_alias_list);
  free(a);
}

static void free_command(command *cmd) {
  free(cmd->name);
  string_list_free_all(cmd->before_args);
  string_list_free_all(cmd->after_args);
  string_list_free_all(cmd->name_aliases);
  alias_list_free_all(cmd->global_alias_list);
  alias_list_free_all(cmd->alias_list);
  free(cmd);
}

static string_list *string_list_last(string_list *list) {
  while (list && list->next) {
    list = list->next;
  }

  return list;
}

string_list *string_list_append(string_list *list, char *str) {
  string_list *last = string_list_last(list);

  string_list *new_item = (string_list *)malloc(sizeof(string_list));
  new_item->str = str;
  new_item->next = NULL;

  if (last) {
    last->next = new_item;
    return list;
  }

  return new_item;
}


alias_list *alias_list_append(alias_list *list, alias *a) {
  alias_list *new_item = (alias_list *)malloc(sizeof(alias_list));

  new_item->alias = a;
  new_item->next = list;

  return new_item;
}

void alias_list_free_all(alias_list *list) {
  alias_list *item;

  while ((item = list)) {
    list = item->next;
    free_alias(item->alias);
    free(item);
  }
}

global_alias_list *global_alias_list_prepend(global_alias_list *globals,
                                             alias_list *aliases) {
  global_alias_list *new_item =
      (global_alias_list *)malloc(sizeof(global_alias_list));

  new_item->alias_list = aliases;
  new_item->next = globals;
  return new_item;
}

void global_alias_list_delete(global_alias_list *globals) {
  global_alias_list *item;
  while ((item = globals)) {
    globals = item->next;
    free(item);
  }
}

command_list *command_list_append(command_list *list, command *cmd) {
  command_list *new_item = (command_list *)malloc(sizeof(command_list));

  new_item->command = cmd;
  new_item->next = list;
  return new_item;
}

void command_list_free_all(command_list *list) {
  command_list *item;

  while ((item = list)) {
    list = item->next;
    free_command(item->command);
    free(item);
  }
}
