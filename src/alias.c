#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cmdalias.h"


static command *get_cmd(command_list *list, const char *cmd) {
	while (list) {
		if (0 == strcmp(cmd, list->command->name)) {
			return list->command;
		}
		list = list->next;
	}

	return NULL;
}

static alias *get_alias(alias_list *list, const char *name) {
	while (list) {
		string_list *name_list = list->alias->names;
		while (name_list) {
			if (0 == strcmp(name, name_list->data)) {
				return list->alias;
			}
			name_list = name_list->next;
		}
		list = list->next;
	}

	return NULL;
}

int alias_execute(command_list *commands, int argc, char **argv) {
	int args_c = 0;
	char *args[100];
	int i;
	alias *a;
	alias_list *aliases = NULL;
	alias_list *global  = NULL;

	command *cmd = get_cmd(commands, argv[0]);
	if (cmd) {
		aliases = cmd->aliases;
		global  = cmd->global;
	}
	args[args_c++] = argv[0];

	for (i = 1; i < argc; i++) {
		a = get_alias(aliases, argv[i]);
		if (a) {
			string_list *str_list = a->substitutes;

			if (a->is_cmd) {
				args_c = 0;
			}

			while (str_list) {
				args[args_c++] = str_list->data;
				str_list = str_list->next;
			}

			aliases = a->subaliases;
		} else if ((a = get_alias(global, argv[i]))) {
			string_list *str_list = a->substitutes;
			while (str_list) {
				args[args_c++] = str_list->data;
				str_list = str_list->next;
			}
			aliases = NULL;
		} else {
			args[args_c++] = argv[i];
			aliases = NULL;
		}
	}

	args[args_c++] = NULL;

#if CMDALIAS_DEBUG
	debug_msg("Executing:\n");
	for (i=0; args[i] != NULL; i++) {
		debug_msg("\t[%d] %s\n", i, args[i]);
	}
#endif

	return execvp(args[0], args);
}
