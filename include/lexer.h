#ifndef _CONFIG_LEXER_H_
#define _CONFIG_LEXER_H_

#include <stdlib.h>
#include "cmdalias.h"

extern struct cmdalias_config_bufstack_t *curbs;

int cmdalias_config_pushfile(const char *);
int cmdalias_config_popfile(void);
int yylex (void);

void cmdalias_config_init(cmdalias_config *);
int cmdalias_config_load(const char *, cmdalias_config *);
void cmdalias_config_destroy(cmdalias_config *);
char *cmdalias_config_get_current_filename(void);

#endif /* _CONFIG_LEXER_H_ */
