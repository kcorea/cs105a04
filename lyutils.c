
#include <assert.h>
#include <ctype.h>

#include "auxlib.h"
#include "lyutils.h"

cmdtree yyparse_cmdtree = NULL;
bool yyparse_cmdbg_flag = false;

void scanner_badchar (unsigned char bad) {
   char char_rep[16];
   sprintf (char_rep, isgraph ((int) bad) ? "%c" : "\\%03o", bad);
   errprintf ("%: invalid character (%s)\n", char_rep);
}

void scanner_badtoken (char *lexeme) {
   errprintf ("%: invalid token (%s)\n", lexeme);
}

void yyerror (char *message) {
   errprintf ("%: %s\n", message);
}

cmdtree new_parseroot (void) {
   yyparse_cmdtree = new_cmdtree (CMD_ROOT, "");
   return yyparse_cmdtree;
}

cmdtree new_parseroot1 (int symbol) {
   cmdtree newroot = new_cmdtree (symbol, "");
   return newroot;
}

int yylval_token (int symbol) {
   yylval = new_cmdtree (symbol, yytext);
   DEBUGF ('t', "sym=%d, lex=%s\n", symbol, yytext);
   return symbol;
}
