#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>

#include "cmdalias.h"
#include "lexer.h"

static void display_usage() {
	puts("Usage: ./cmdalias [OPTION] -- <command> [args...] ");
	puts("  -c, --config=CONF       Configuration file or directory (default: ~/.cmdalias)");
	puts("  -i, --init              Use this option to add in your bash profile and initialize your aliases");
	puts("                          source<(cmdalias -i)");
	puts("  -h, --help              Display this help");
	puts("  -V, --version           Display version");
	puts("      --check-config      Check the configuration file");
	puts("");
	exit(EXIT_FAILURE);
}

static void display_version() {
	puts("CmdAlias " CMDALIAS_VERSION " (c)2017 Pierrick Charron - Adoy.net");
	exit(EXIT_FAILURE);
}

static void check_config(const char *configFile) {
	int exit_status;
	command_list *commands = NULL;
	if (config_load(configFile, &commands)) {
		puts("Syntax OK");
		exit_status = EXIT_SUCCESS;
	} else {
		exit_status = EXIT_FAILURE;
	}
	command_list_free_all(commands);
	exit(exit_status);
}

static void cmdalias_bash_init(const char *configFile) {
	int exit_status;
	command_list *commands = NULL;
	string_list *name_aliases = NULL;

	if (config_load(configFile, &commands)) {
		while (commands) {
			if (configFile) {
				fprintf(stdout, "alias %s=\"cmdalias -c %s -- %s\";\n", commands->command->name, configFile, commands->command->name);
			} else {
				fprintf(stdout, "alias %s=\"cmdalias -- %s\";\n", commands->command->name, commands->command->name);
			}
			name_aliases = commands->command->name_aliases;
			while (name_aliases) {
				if (configFile) {
					fprintf(stdout, "alias %s=\"cmdalias -c %s -- %s\";\n", name_aliases->data, configFile, commands->command->name);
				} else {
					fprintf(stdout, "alias %s=\"cmdalias -- %s\";\n", name_aliases->data, commands->command->name);
				}
				name_aliases = name_aliases->next;
			}
			commands = commands->next;
		}

		exit_status = EXIT_SUCCESS;
	} else {
		exit_status = EXIT_FAILURE;
	}

	command_list_free_all(commands);
	exit(exit_status);
}

static int cmdalias(const char *configFile, int argc, char **argv) {
	int exit_status;
	command_list *commands = NULL;
	if (config_load(configFile, &commands)) {
		exit_status = argc == 1 ? execvp(argv[0], argv) : alias_execute(commands, argc, argv);
	} else {
		exit_status = EXIT_FAILURE;
	}

	command_list_free_all(commands);
	return exit_status;
}

int main(int argc, char **argv)
{
	int longIndex, opt;
	static const char *optString = "c:h?Vi";
	static const struct option longOpts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'V'},
		{ "config", required_argument, NULL, 'c' },
		{ "check-config", no_argument, NULL, 0 },
		{ "init", no_argument, NULL, 0 },
		{ NULL, no_argument, NULL, 0 }
	};

	struct {
		char *config_file; /* -c */
		int check_config;  /* --check-config */
		int init;          /* --init */
	} cmdalias_args;

	cmdalias_args.config_file  = NULL;
	cmdalias_args.check_config = 0;
	cmdalias_args.init         = 0;

	opt = getopt_long(argc, argv, optString, longOpts, &longIndex);

	while ( opt != -1 ) {
		switch ( opt ) {
			case 'c':
				cmdalias_args.config_file = optarg;
				break;
			case 'h':   /* fall-through is intentional */
			case '?':
				display_usage();
				break;
			case 'i':
				cmdalias_args.init = 1;
				break;
			case 'V':
				display_version();
				break;
			case 0:
				if (strcmp("check-config", longOpts[longIndex].name) == 0) {
					cmdalias_args.check_config = 1;
				} else if (strcmp("init", longOpts[longIndex].name) == 0) {
					cmdalias_args.init = 1;
				}
				break;
			default:
				/* You won't actually get here. */
				break;
		}

		opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
	}

	if (cmdalias_args.check_config) {
		check_config(cmdalias_args.config_file);
	}

	if (cmdalias_args.init) {
		cmdalias_bash_init(cmdalias_args.config_file);
	}

	if (0 == argc - optind) {
		display_usage();
		exit(EXIT_SUCCESS);
	}

	exit(cmdalias(cmdalias_args.config_file, argc - optind, argv + optind));
}
