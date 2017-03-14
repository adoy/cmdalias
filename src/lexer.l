%option never-interactive case-insensitive warn nodefault noinput nounput yylineno noyywrap

%{
#include <string.h>

#include "cmdalias.h"
#include "lexer.h"
#include "parser.h"

struct cmdalias_config_bufstack_t {
	struct cmdalias_config_bufstack_t *prev;
	YY_BUFFER_STATE bs;
	int lineno;
	char *filename;
	FILE *f;
} *curbs = 0;

int cmdalias_config_pushfile(const char *filename) {

	FILE *f = fopen(filename, "r");
	struct cmdalias_config_bufstack_t *bs = (struct cmdalias_config_bufstack_t *) malloc(sizeof(struct cmdalias_config_bufstack_t));

	if (!f || !bs) return 0;

	debug_msg("Loading config file: %s\n", filename);

	if (curbs) curbs->lineno = yylineno;
	bs->prev = curbs;
	bs->bs = yy_create_buffer(f, YY_BUF_SIZE);
	bs->f = f;
	bs->filename = strdup(filename);
	yy_switch_to_buffer(bs->bs);

	curbs = bs;
	yylineno = 1;
	return 1;
}

int cmdalias_config_popfile(void) {
	struct cmdalias_config_bufstack_t *bs = curbs;
	struct cmdalias_config_bufstack_t *prevbs;


	if (!bs) return 0;

	fclose(bs->f);
	yy_delete_buffer(bs->bs);
	prevbs = bs->prev;
	free(bs->filename);
	free(bs);

	if (!prevbs) {
		curbs = NULL;
		return 0;
	}

	yy_switch_to_buffer(prevbs->bs);
	curbs = prevbs;

	yylineno = curbs->lineno;
	return 1;
}

char *cmdalias_config_get_current_filename(void) {
	if (curbs) {
		return curbs->filename;
	}

	return NULL;
}

%}

%%

<<EOF>>				{ if (!cmdalias_config_popfile()) yyterminate(); }
\"(\\.|[^\\"])*\"	{ yytext[yyleng-1] = 0; yylval.str = strdup(yytext+1); return T_STR; }
INCLUDE				{ return T_INCLUDE; }
[-_0-9A-Za-z]+		{ yylval.str = strdup(yytext); return T_NAME; }
[ \t\r\n]+			{ /* IGNORE */ }
#.*					{ /* COMMENTS */ }
.					{ return yytext[0]; }

%%
