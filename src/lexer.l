%option never-interactive case-insensitive warn noinput nounput yylineno noyywrap

%{
#include <string.h>

#include "cmdalias.h"
#include "lexer.h"
#include "parser.h"

#define BUFFER_SIZE 1024

struct config_bufstack_t {
	struct config_bufstack_t *prev;
	YY_BUFFER_STATE bs;
	int lineno;
	char *filename;
	FILE *f;
} *curbs = 0;

int config_pushfile(const char *filename) {

	FILE *f = fopen(filename, "r");
	struct config_bufstack_t *bs = (struct config_bufstack_t *) malloc(sizeof(struct config_bufstack_t));

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

int config_popfile(void) {
	struct config_bufstack_t *bs = curbs;
	struct config_bufstack_t *prevbs;


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

char *config_get_current_filename(void) {
	if (curbs) {
		return curbs->filename;
	}

	return NULL;
}

%}

%x IN_STRING
%x IN_SUBCMD

NAME 		[-_0-9A-Za-z]+
WHITESPACE 	[ \t\r\n]+
TOKENS      [;={}!*,]
ANYCHAR    	.

%%

	int string_size = 0;
	char string_buf[BUFFER_SIZE];
	char *string_buf_ptr = string_buf;


<INITIAL><<EOF>>		{ if (!config_popfile()) yyterminate(); }
<INITIAL>\"             { string_buf[0] = '\0'; BEGIN(IN_STRING); }
<INITIAL>`              { string_buf[0] = '\0'; BEGIN(IN_SUBCMD); }
<INITIAL>INCLUDE		{ return T_INCLUDE; }
<INITIAL>{NAME}			{ yylval.str = strdup(yytext); return T_NAME; }
<INITIAL>{WHITESPACE}	{ /* IGNORE */ }
<INITIAL>#.*			{ /* COMMENTS */ }
<INITIAL>{TOKENS}		{ return yytext[0]; }
<INITIAL>{ANYCHAR}		{ yyerror(NULL, "Unexpected character in input: '%c' (ASCII=%d) state=%d", yytext[0], yytext[0], YYSTATE); exit(EXIT_FAILURE); }

<IN_STRING,IN_SUBCMD><<EOF>>  { yyerror(NULL, "Unterminated string"); exit(EXIT_FAILURE); }
<IN_STRING>[^\\"]+	{
	if (yyleng + string_size > BUFFER_SIZE) {
		yyerror(NULL, "String too long max cmd string buffer size is %d", BUFFER_SIZE);
		exit(EXIT_FAILURE);
	}
	strncpy(string_buf_ptr, yytext, BUFFER_SIZE - string_size);
	string_size += yyleng;
	string_buf_ptr += yyleng;
}
<IN_STRING>\\.     {
	if (yyleng + 1 > BUFFER_SIZE) {
		yyerror(NULL, "String too long max cmd string buffer size is %d", BUFFER_SIZE);
		exit(EXIT_FAILURE);
	}
	*string_buf_ptr = yytext[1];
	string_size++;
	string_buf_ptr++;
}
<IN_STRING>\" 		{
	BEGIN(INITIAL);
	yylval.str = strndup(string_buf, string_size);
	string_buf_ptr = string_buf;
	string_size = 0;
	return T_STR;
}
<IN_SUBCMD>[^\\`]+	{
	if (yyleng + string_size > BUFFER_SIZE) {
		yyerror(NULL, "String too long max cmd string buffer size is %d", BUFFER_SIZE);
		exit(EXIT_FAILURE);
	}
	strncpy(string_buf_ptr, yytext, BUFFER_SIZE - string_size);
	string_size += yyleng;
	string_buf_ptr += yyleng;
}
<IN_SUBCMD>\\.     {
	if (yyleng + 1 > BUFFER_SIZE) {
		yyerror(NULL, "String too long max cmd string buffer size is %d", BUFFER_SIZE);
		exit(EXIT_FAILURE);
	}
	*string_buf_ptr = yytext[1];
	string_size++;
	string_buf_ptr++;
}
<IN_SUBCMD>\` 		{
	BEGIN(INITIAL);
	yylval.str = strndup(string_buf, string_size);
	string_buf_ptr = string_buf;
	string_size = 0;
	return T_CMD;
}
%%
