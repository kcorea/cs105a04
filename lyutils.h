#ifndef __LYUTILS_H__
#define __LYUTILS_H__

// flex and bison interface utility

#include <stdio.h>

#include "cmdtree.h"

#define YYEOF  0

extern cmdtree yyparse_cmdtree;
extern bool yyparse_cmdbg_flag;
extern char *yytext;
extern int yy_flex_debug;
extern int yydebug;

int yylex (void);
int yyparse (void);
void yyerror (char *message);
const char *get_yytname (int symbol);

void scanner_badchar (unsigned char bad);
void scanner_badtoken (char *lexeme);

cmdtree new_parseroot (void);
cmdtree new_parseroot1 (int symbol);
int yylval_token (int symbol);

#define YYSTYPE cmdtree
#include "yyparse.h"

#endif
