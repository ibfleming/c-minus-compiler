# Makefile for C- Scanner/Parser

PROJ = c-
ASGN = parser
CC = g++ -pedantic -g 
LCMP = flex
YCMP = bison -v -t -d

SRCS = $(ASGN).y $(ASGN).l
HDRSRCS = yyerror.cpp AST.cpp symbolTable.cpp semantics.cpp routines.cpp main.c
HDRS = yyerror.hpp TokenData.h AST.hpp symbolTable.hpp scope.hpp semantics.hpp routines.hpp
HDROBJS = yyerror.o AST.o symbolTable.o semantics.o routines.o main.o
OBJS = lex.yy.o $(ASGN).tab.o
DOCS = hw5.pdf

$(PROJ) : $(OBJS) $(HDROBJS)
	       $(CC) $(OBJS) $(HDROBJS) -o $(PROJ)

$(HDROBJS) : $(HDRS)
			    $(CC) -c $(HDRSRCS)

lex.yy.c : $(ASGN).l $(ASGN).tab.h $(HDRS)
			  $(LCMP) $(ASGN).l

$(ASGN).tab.h $(ASGN).tab.c : $(ASGN).y
				                  $(YCMP) $(ASGN).y

clean : 
		rm -f *~ $(OBJS) $(HDROBJS) lex.yy.c $(ASGN).tab.h $(ASGN).tab.c $(ASGN).output c-

tar : $(HDRS) $(SRCS) $(HDRSRCS) makefile
		tar -cvf hw6.tar $(HDRS) $(SRCS) $(HDRSRCS) makefile