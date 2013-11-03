
GCC       = gcc -g -O0 -Wall -Wextra -std=gnu99

HSOURCES = auxlib.h cmdtree.h lyutils.h cmdtree.rep.h
CSOURCES = auxlib.c cmdtree.c lyutils.c main.c
LSOURCE  = lexer.l
YSOURCE  = parser.y
ETCSRC   = README Makefile
CLGEN    = yylex.c
HYGEN    = yyparse.h
CYGEN    = yyparse.c
CGENS    = ${CLGEN} ${CYGEN}
ALLGENS  = ${CLGEN} ${CYGEN} ${HYGEN}
EXECBIN  = mysh
ALLCSRC  = ${CSOURCES} ${CGENS}
OBJECTS  = ${ALLCSRC:.c=.o}
LREPORT  = yylex.output
YREPORT  = yyparse.output
ALLSRC   = ${ETCSRC} ${YSOURCE} ${LSOURCE} ${HSOURCES} ${CSOURCES}


all : ${EXECBIN}

${EXECBIN} : ${ALLGENS} ${OBJECTS}
	${GCC} -o $@ ${OBJECTS}

%.o : %.c
	${GCC} -c $<

${CLGEN} : ${LSOURCE} ${HYGEN}
	flex -o${CLGEN} ${LSOURCE} 2>${LREPORT}
	- grep -v '^  ' ${LREPORT}
	- (perl -e 'print "="x65,"\n"'; cat lex.backup) >>${LREPORT}
	- rm lex.backup

${CYGEN} ${HYGEN} : ${YSOURCE}
	bison -dtv -o${CYGEN} ${YSOURCE}

clean :
	- rm ${OBJECTS} ${ALLGENS} ${LREPORT} ${YREPORT}

spotless : clean
	- rm ${EXECBIN}

