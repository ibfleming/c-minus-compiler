# Makefile for C- Scanner/Parser

PROJ = c-
ASGN = parser
CC = g++ -pedantic -g 
LCMP = flex
YCMP = bison -v -t -d

SRCS = $(ASGN).y $(ASGN).l
HDRSRCS = yyerror.cpp AST.cpp symbolTable.cpp semantics.cpp routines.cpp code_gen.cpp code_gen_routines.cpp code_gen_special.cpp emitcode.cpp main.c
HDRS = yyerror.hpp TokenData.h AST.hpp symbolTable.hpp scope.hpp semantics.hpp routines.hpp emitcode.h code_gen.hpp
HDROBJS = yyerror.o AST.o symbolTable.o semantics.o routines.o code_gen.o code_gen_routines.o code_gen_special.o emitcode.o main.o
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
		rm -f *~ $(OBJS) $(HDROBJS) lex.yy.c $(ASGN).tab.h $(ASGN).tab.c $(ASGN).output c- *.tm

tar : $(HDRS) $(SRCS) $(HDRSRCS) makefile
		tar -cvf hw7.tar $(HDRS) $(SRCS) $(HDRSRCS) makefile