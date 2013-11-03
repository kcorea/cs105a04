%{

#include <stdbool.h>

#include "lyutils.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token CMD_ROOT WORD PROG ARGS SIMPL PIPE 
%token REIN REOUT LEFT RIGHT INVOC
%token '(' ')' '*'
%token ';' '&'  

%nonassoc '\n'
%nonassoc '&'
%nonassoc ';'
%left '|' 
%left '>' '<' 



%start start

%%

start      : command '\n'             { yyparse_cmdtree = $1; YYACCEPT; } 
           ;


command    : cmd_list '&'             { yyparse_cmdbg_flag = true;  }
           | cmd_list                 { yyparse_cmdbg_flag = false; }
           ;


cmd_list   : cmd_list simple_cmd     { $$ = adopt1 ($1, $2); }
           | cmd_list pipe_cmd       { $$ = adopt1 ($1, $2); }
           | /* NULL */               { $$ = new_parseroot (); }
           ;


pipe_cmd   : prog_invoc input_red '|' prog_invoc output_red     
             { cmdtree left = adopt2 (new_parseroot1(LEFT), $1, $2); 
               cmdtree right = adopt2 (new_parseroot1(RIGHT), $4, $5); 
               $$ = adopt2sym ($3, left, right, PIPE); }

           | prog_invoc input_red '|' prog_invoc
             { cmdtree left = adopt2 (new_parseroot1(LEFT), $1, $2); 
               cmdtree right = adopt1 (new_parseroot1(RIGHT), $4); 
               $$ = adopt2sym ($3, left, right, PIPE); }

           | prog_invoc '|' prog_invoc output_red
             { cmdtree left = adopt1 (new_parseroot1(LEFT), $1); 
               cmdtree right = adopt2 (new_parseroot1(RIGHT), $3, $4); 
               $$ = adopt2sym ($2, left, right, PIPE); }

           | prog_invoc '|' prog_invoc
             { cmdtree left = adopt1 (new_parseroot1(LEFT), $1); 
               cmdtree right = adopt1 (new_parseroot1(RIGHT), $3); 
               $$ = adopt2sym ($2, left, right, PIPE); }
           ; 


simple_cmd : prog_invoc input_red output_red
             { $$ = adopt2 (new_parseroot1 (SIMPL), $1, $2);
               $$ = adopt1 ($$, $3); }

           | prog_invoc output_red input_red
             { $$ = adopt2 (new_parseroot1 (SIMPL), $1, $3);
               $$ = adopt1 ($$, $2); }

           | prog_invoc input_red 
             { $$ = adopt2 (new_parseroot1 (SIMPL), $1, $2); }

           | prog_invoc output_red 
             { $$ = adopt2 (new_parseroot1 (SIMPL), $1, $2); }

           | prog_invoc 
             { $$ = adopt1 (new_parseroot1 (SIMPL), $1); }
           ;


prog_invoc : prog args
             { $$ = adopt2(new_parseroot1 (INVOC), $1, $2); }
           | prog 
             { $$ = adopt1(new_parseroot1 (INVOC), $1); }
           ;


output_red : '>' WORD       { $$ = adopt1sym($1, $2, REOUT); }
           ;


input_red  : '<' WORD       { $$ = adopt1sym($1, $2, REIN); }
           ;


prog       : WORD           { $$ = adopt1(new_parseroot1 (PROG), $1); }
           ;


args       : args WORD      { $$ = adopt1 ($1, $2); }
           | WORD           { $$ = adopt1 (new_parseroot1 (ARGS), $1); }
           ;
%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

