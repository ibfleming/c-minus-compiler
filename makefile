# Makefile for C- Scanner/Parser

PROJ = c-
ASGN = parser
CC = g++
LCMP = flex
YCMP = bison -v -t -d

SRCS = $(ASGN).y $(ASGN).l
HDRSRCS = AST.c
HDRS = TokenData.h AST.h
HDROBJS = AST.o
OBJS = lex.yy.o $(ASGN).tab.o
DOCS = hw2.pdf

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
		tar -cvf hw2.tar $(HDRS) $(SRCS) $(HDRSRCS) $(DOCS) makefile