%{

#include "stdlib.h"

#include "auxlib.h"
#include "lyutils.h"

%}

%option 8bit
%option always-interactive
%option backup
%option debug
%option ecs
%option nodefault
%option nounput
%option noyywrap
%option perf-report
%option verbose

WORD     [a-zA-Z0-9\/\.-]+

%%

[ \t]      { /* ignore whitespace  */ }
"\n"       { return yylval_token ('\n'); }

"exit"     { exit (get_exitstatus()); }
{WORD}     { return yylval_token (WORD); }

"&"        { return yylval_token ('&'); }
"("        { return yylval_token ('('); }
")"        { return yylval_token (')'); }
"*"        { return yylval_token ('*'); }
";"        { return yylval_token (';'); }
"|"        { return yylval_token ('|'); }
">"        { return yylval_token ('>'); }
"<"        { return yylval_token ('<'); }

.          { scanner_badchar (*yytext); }

%%
