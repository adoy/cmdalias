#ifndef _CONFIG_LEXER_H_
#define _CONFIG_LEXER_H_

#include <stdlib.h>
#include "cmdalias.h"

extern struct config_bufstack_t *curbs;

int config_pushfile(const char *);
int yylex (void);
void yyerror(command_list **, const char *s, ...);

int config_load(const char *, command_list **);
char *config_get_current_filename(void);

#endif /* _CONFIG_LEXER_H_ */
