# Makefile for C- Scanner/Parser

PROJ = c-
ASGN = parser
CC = g++
LCMP = flex
YCMP = bison -v -t -d

SRCS = $(ASGN).y $(ASGN).l
HDRS = scanType.h
OBJS = lex.yy.o $(ASGN).tab.o
DOCS = hw1.pdf

$(PROJ) : $(OBJS)
	       $(CC) $(OBJS) -o $(PROJ)

lex.yy.c : $(ASGN).l $(ASGN).tab.h $(HDRS)
			  $(LCMP) $(ASGN).l

$(ASGN).tab.h $(ASGN).tab.c : $(ASGN).y
				                  $(YCMP) $(ASGN).y

clean : 
		rm -f *~ $(OBJS) lex.yy.c $(ASGN).tab.h $(ASGN).tab.c $(ASGN).output

tar : $(HDRS) $(SRCS) makefile
		tar -cvf hw1.tar $(HDRS) $(SRCS) $(DOCS) makefile