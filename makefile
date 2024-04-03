# Makefile for C- Scanner/Parser

PROJ = c-
ASGN = parser
CC = g++ -g
LCMP = flex
YCMP = bison -v -t -d

SRCS = $(ASGN).y $(ASGN).l
HDRSRCS = AST.cpp symbolTable.cpp semantics.cpp routines.cpp
HDRS = TokenData.h AST.h symbolTable.h scope.h semantics.h routines.h
HDROBJS = AST.o symbolTable.o semantics.o routines.o
OBJS = lex.yy.o $(ASGN).tab.o
DOCS = hw4.pdf

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
		tar -cvf hw4.tar $(HDRS) $(SRCS) $(HDRSRCS) $(DOCS) makefile