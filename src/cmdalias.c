#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "bash_autocomplete.c"
#include "cmdalias.h"
#include "lexer.h"

static void display_usage() {
  puts("Usage: cmdalias [OPTION] -- <command> [args...] ");
  puts("  -c, --config=CONF       Configuration file or directory (default: "
       "~/.cmdalias)");
  puts("  -i, --init              Use this option to add in your bash profile "
       "and initialize your aliases");
  puts("                          source<(cmdalias -i)");
  puts("  -h, --help              Display this help");
  puts("  -d, --debug             Debug mode");
  puts("  -e, --expend            Expend command");
  puts("  -V, --version           Display version");
  puts("      --check-config      Check the configuration file");
  puts("      --complete[=SHELL]  Use this option to add in your bash profile "
       "and initialize autocomplete (default: bash)");
  puts("                          source<(cmdalias --complete=bash)");
  puts("");
  exit(EXIT_FAILURE);
}

static void display_version() {
  puts("cmdalias " CMDALIAS_VERSION
       " (c)2017-2019 Pierrick Charron - Adoy.net");
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
    command_list *current_cmd = commands;
    while (current_cmd) {
      name_aliases = current_cmd->command->name_aliases;
      if (!name_aliases) {
        if (configFile) {
          fprintf(stdout, "alias %s=\"cmdalias -c %s -- %s\";\n",
                  current_cmd->command->name, configFile,
                  current_cmd->command->name);
        } else {
          fprintf(stdout, "alias %s=\"cmdalias -- %s\";\n",
                  current_cmd->command->name, current_cmd->command->name);
        }
      } else {
        while (name_aliases) {
          if (configFile) {
            fprintf(stdout, "alias %s=\"cmdalias -c %s -- %s\";\n",
                    name_aliases->str, configFile, name_aliases->str);
          } else {
            fprintf(stdout, "alias %s=\"cmdalias -- %s\";\n", name_aliases->str,
                    name_aliases->str);
          }
          name_aliases = name_aliases->next;
        }
      }
      current_cmd = current_cmd->next;
    }
    exit_status = EXIT_SUCCESS;
  } else {
    exit_status = EXIT_FAILURE;
  }

  command_list_free_all(commands);
  exit(exit_status);
}

static void cmdalias_complete_bash(const char *configFile) {
  int exit_status;
  command_list *commands = NULL;
  string_list *name_aliases = NULL;

  if (config_load(configFile, &commands)) {

    command_list *current_cmd = commands;
    fprintf(stdout, "%.*s\n", autocomplete_bash_len, autocomplete_bash);

    while (current_cmd) {
      name_aliases = current_cmd->command->name_aliases;
      if (!name_aliases) {
        fprintf(stdout, "complete -o default -F _cmdalias %s;\n",
                current_cmd->command->name);

      } else {
        while (name_aliases) {
          fprintf(stdout, "complete -o default -F _cmdalias %s;\n",
                  name_aliases->str);
          name_aliases = name_aliases->next;
        }
      }
      current_cmd = current_cmd->next;
    }
    exit_status = EXIT_SUCCESS;
  } else {
    exit_status = EXIT_FAILURE;
  }

  command_list_free_all(commands);
  exit(exit_status);
}

static int cmdalias(const char *configFile, int argc, char **argv, int debug) {
  int exit_status;
  command_list *commands = NULL;
  if (config_load(configFile, &commands)) {
    alias_substitution_result *result =
        alias_substitution(commands, argc, argv);

    if (1 == debug) {
      for (int i = 0; i < result->argc - 1; i++) {
        fprintf(stdout, result->argv[i] ? result->argv[i] : "|");
        if (i < result->argc - 2) {
          fprintf(stdout, " ");
        }
      }
      fprintf(stdout, "\n");
      exit_status = EXIT_SUCCESS;
    } else if (2 == debug) {
      fprintf(stdout, "Executing:\n");
      for (int i, j = 0; i < result->argc - 1; i++) {
        if (NULL != result->argv[i]) {
          fprintf(stdout, "\t[%d] %s\n", j, result->argv[i]);
          j++;
        } else {
          j = 0;
          fprintf(stdout, " |\n");
        }
      }
      exit_status = EXIT_SUCCESS;
    } else {
      int index = 0;
      int pipefd[2];
      int fd_in = STDIN_FILENO;

      for (int i = 0; i < result->argc; i++) {
        if (NULL == result->argv[i]) {
          pipe(pipefd);
          // 1 = write end of the pipe
          // 0 = read end of the pipe

          if (fork() == 0) {
            dup2(fd_in, STDIN_FILENO);
            if (i < result->argc - 1) {
              dup2(pipefd[1], STDOUT_FILENO);
            }
            close(pipefd[0]);

            execvp(result->argv[index], result->argv + index);
            fprintf(stderr, "cmdalias: %s: %s\n", result->argv[index],
                    strerror(errno));
            exit(EXIT_FAILURE);
          }

          wait(NULL);

          close(pipefd[1]);
          fd_in = pipefd[0];

          index = i + 1;
        }
      }
    }
    free(result);
  } else {
    exit_status = EXIT_FAILURE;
  }

  command_list_free_all(commands);
  return exit_status;
}

int main(int argc, char **argv) {
  int longIndex, opt;
  static const char *optString = "c:h?Vide";
  static const struct option longOpts[] = {
      {"help", no_argument, NULL, 'h'},
      {"version", no_argument, NULL, 'V'},
      {"config", required_argument, NULL, 'c'},
      {"debug", no_argument, NULL, 'd'},
      {"expend", no_argument, NULL, 'e'},
      {"check-config", no_argument, NULL, 0},
      {"complete", optional_argument, NULL, 0},
      {"init", no_argument, NULL, 0},
      {NULL, no_argument, NULL, 0}};

  struct {
    char *config_file; /* -c */
    int check_config;  /* --check-config */
    int init;          /* --init */
    int expend;        /* --expend */
    char *complete;    /* --complete */
  } cmdalias_args;

  cmdalias_args.config_file = NULL;
  cmdalias_args.check_config = 0;
  cmdalias_args.init = 0;
  cmdalias_args.expend = 0;
  cmdalias_args.complete = NULL;

  opt = getopt_long(argc, argv, optString, longOpts, &longIndex);

  while (opt != -1) {
    switch (opt) {
    case 'c':
      cmdalias_args.config_file = optarg;
      break;
    case 'd':
      cmdalias_args.expend = 2;
      break;
    case 'e':
      cmdalias_args.expend = 1;
      break;
    case 'h': /* fall-through is intentional */
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
      } else if (strcmp("debug", longOpts[longIndex].name) == 0) {
        cmdalias_args.expend = 2;
      } else if (strcmp("expend", longOpts[longIndex].name) == 0) {
        cmdalias_args.expend = 1;
      } else if (strcmp("complete", longOpts[longIndex].name) == 0) {
        cmdalias_args.complete = optarg ? optarg : "bash";
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

  if (cmdalias_args.complete) {
    if (strcmp("bash", cmdalias_args.complete) == 0) {
      cmdalias_complete_bash(cmdalias_args.config_file);
    } else {
      fprintf(stderr, "Unsupported shell type \"%s\"\n",
              cmdalias_args.complete);
      exit(EXIT_FAILURE);
    }
  }

  if (0 == argc - optind) {
    display_usage();
    exit(EXIT_SUCCESS);
  }

  exit(cmdalias(cmdalias_args.config_file, argc - optind, argv + optind,
                cmdalias_args.expend));
}
