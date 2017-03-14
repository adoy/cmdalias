%{

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>


#include "cmdalias.h"
#include "lexer.h"

#define YYERROR_VERBOSE
#define	YYPARSE_PARAM config

void yyerror(const char *s, ...) {
	extern int yylineno;

	va_list ap;
	va_start(ap, s);

	fprintf(stderr, "Error: ");
	vfprintf(stderr, s, ap);
	fprintf(stderr, " in '%s' on line %d\n", cmdalias_config_get_current_filename(), yylineno);
	fflush(stderr);
}

int is_dir(const char *path) {
	struct stat st;

	if (lstat(path, &st) == -1) {
		return -1;
	}

	return S_ISDIR(st.st_mode);
}

int cmdalias_config_pushdir(const char *dirname) {
	struct dirent *dent;
	DIR *dir;
	char fn[FILENAME_MAX];
	int len = strlen(dirname);

	if (len >= FILENAME_MAX - 1) {
		/* Name too long */
		return 0;
	}

	strcpy(fn, dirname);
	fn[len++] = '/';

	if (!(dir = opendir(dirname))) {
		/* Can't open directory */
		return 0;
	}

	while ((dent = readdir(dir))) {
		if (dent->d_name[0] == '.') continue;
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;

		strncpy(fn + len, dent->d_name, FILENAME_MAX - len);

		if (is_dir(fn)) {
			cmdalias_config_pushdir(fn);
			continue;
		}

		cmdalias_config_pushfile(fn);
	}

	if (dir) closedir(dir);

	return 1;
}
%}

%union {
	char *str;
	struct string_list_t *str_list;
	struct alias_t *alias;
	struct alias_list_t *alias_list;
	struct command_t *cmd;
	struct command_list_t *cmd_list;
	cmdalias_bool mbool;
}

%token <str>  T_NAME "Name (T_NAME)"
%token <str>  T_STR  "String (T_STR)"
%token T_INCLUDE     "Include (T_INCLUDE)"

%type <str> string
%type <str_list> string_list alias_name_list
%type <alias> alias
%type <alias_list> global_alias_list_or_empty alias_list_or_empty alias_list
%type <mbool> is_cmd

%%

configs_or_empty:
		configs
	|	/* empty */
;

configs:
		configs config
	|	configs include
	|	config
	|	include
;

include:
		T_INCLUDE T_STR ';' {
			if (!(is_dir($2) ? cmdalias_config_pushdir($2) : cmdalias_config_pushfile($2))) {
				yyerror("Unable to load %s", $2);
			}
			free($2);
		}
;

config:
		T_NAME '{' global_alias_list_or_empty alias_list_or_empty '}' end {
			cmdalias_config *cfg = (cmdalias_config *) config;

			command *cmd = (command *) malloc(sizeof(command));
			cmd->name = $1;
			cmd->global = $3;
			cmd->aliases = $4;

			cfg->commands = command_list_append(cfg->commands, cmd);
		}

;

global_alias_list_or_empty:
		'*' '{' alias_list_or_empty '}' { $$ = $3; }
	|	/* empty */ { $$ = NULL; }
;

alias_list_or_empty:
		alias_list { $$ = $1; }
	|	/* empty */ { $$ = NULL; }
;

alias_list:
		alias_list alias { $$ = alias_list_append($1, $2); }
	|	alias { $$ = alias_list_append(NULL, $1); }
;

alias:
		alias_name_list '=' is_cmd string_list ';' {
			$$ = (alias *) malloc(sizeof(alias));
			$$->names		= $1;
			$$->is_cmd		= $3;
			$$->substitutes = $4;
			$$->subaliases  = NULL;
		}
	|	alias_name_list '=' is_cmd string_list '{' alias_list_or_empty '}' end {
			$$ = (alias *) malloc(sizeof(alias));
			$$->names		= $1;
			$$->is_cmd		= $3;
			$$->substitutes = $4;
			$$->subaliases  = $6;
		}
	|  T_NAME '{' alias_list_or_empty '}' end {
			$$ = (alias *) malloc(sizeof(alias));
			$$->names 		= string_list_append(NULL, $1);
			$$->is_cmd		= 0;
			$$->substitutes = string_list_append(NULL, $1);
			$$->subaliases  = $3;
		}

;

alias_name_list:
		alias_name_list ',' T_NAME { $$ = string_list_append($1, $3); }
	|	T_NAME { $$ = string_list_append(NULL, $1); }
;

is_cmd:
		'!' { $$ = 1; }
	|	/* empty */ { $$ = 0; }
;

string_list:
		string_list string { $$ = string_list_append($1, $2); }
	|	string { $$ = string_list_append(NULL, $1); }
;

string:
		T_STR
	|	T_NAME
;

end:
		';'
	|	/* empty */
;

%%

void cmdalias_config_init(cmdalias_config *config) {
	config->commands = NULL;
}

void cmdalias_config_destroy(cmdalias_config *config) {
	command_list_free_all(config->commands);
}

int cmdalias_config_load(const char *path, cmdalias_config *config) {

	if (!path) {
		char buffer[255];
		snprintf(buffer, 255, "%s/.cmdalias", getenv("HOME"));
		path = buffer;
	}

	cmdalias_config_destroy(config);
	cmdalias_config_init(config);

	if (!(is_dir(path) ? cmdalias_config_pushdir(path) : cmdalias_config_pushfile(path))) {
		return 0;
	}

	if (yyparse(config)) {
		return 0;
	}

	return 1;
}
