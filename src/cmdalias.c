#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>

#include "cmdalias.h"
#include "lexer.h"

static void display_usage() {
	puts("Usage: ./cmdalias [OPTION] -- <command> [args...] ");
	puts("  -c, --config=FILE       Use FILE as the configuration file (default: ~/.cmdalias)");
	puts("  -h, --help              Display this help");
	puts("  -V, --version           Display version");
	puts("      --check-config      Check the configuration file");
	puts("");
	exit(EXIT_FAILURE);
}

static void display_version() {
	puts("CmdAlias " CMDALIAS_VERSION " (c)2017 Adoy.net");
	exit(EXIT_FAILURE);
}

static void check_config(const char *configFile) {
	int exit_status;
	cmdalias_config config;
	cmdalias_config_init(&config);
	if (cmdalias_config_load(configFile, &config)) {
		puts("Syntax OK");
		exit_status = EXIT_SUCCESS;
	} else {
		exit_status = EXIT_FAILURE;
	}
	cmdalias_config_destroy(&config);
	exit(exit_status);
}

static int cmdalias(const char *configFile, int argc, char **argv) {
	int exit_status;
	cmdalias_config config;
	cmdalias_config_init(&config);
	if (cmdalias_config_load(configFile, &config)) {

		if (0 == argc) {
			cmdalias_config_destroy(&config);
			display_usage();
		}

		exit_status = argc == 1 ? execvp(argv[0], argv) : alias_execute(&config, argc, argv);
	} else {
		exit_status = EXIT_FAILURE;
	}
	cmdalias_config_destroy(&config);
	return exit_status;
}

int main(int argc, char **argv)
{
	int longIndex, opt;
	static const char *optString = "c:h?V";
	static const struct option longOpts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'V'},
		{ "config", required_argument, NULL, 'c' },
		{ "check-config", no_argument, NULL, 0 },
		{ NULL, no_argument, NULL, 0 }
	};

	struct {
		char *config_file; /* -c */
		int check_config;  /* --check-config */
	} cmdalias_args;

	cmdalias_args.config_file  = NULL;
	cmdalias_args.check_config = 0;

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
			case 'V':
				display_version();
				break;
			case 0:
				if (strcmp("check-config", longOpts[longIndex].name) == 0) {
					cmdalias_args.check_config = 1;
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

	exit(cmdalias(cmdalias_args.config_file, argc - optind, argv + optind));
}
