%option noyywrap
%option yylineno
%{
   #include <string.h>
   #include <bits/stdc++.h>

   #include "TokenData.h"
   #include "AST.h"
   #include "parser.tab.h"

   int lnNum = 1;

   int setData(int lnNum, int tknClass, char *yytext);
   char* workEscChar(char* yytext, int escSlash, char escChar);
%}

%option yylineno

digit [0-9]
number {digit}+
identifier [a-zA-Z][a-zA-Z0-9]*
newline \n
whitespace [ \t]+
true ("true"|"True")
false ("false"|"False")
comment "##"(.*?)

%%                         
"char"                     { return setData(lnNum, CHAR, yytext); }
"bool"                     { return setData(lnNum, BOOL, yytext); }
"int"                      { return setData(lnNum, INT, yytext); }
"if"                       { return setData(lnNum, IF, yytext); }
"then"                     { return setData(lnNum, THEN, yytext); }
"do"                       { return setData(lnNum, DO, yytext); }
"else"                     { return setData(lnNum, ELSE, yytext); }
"while"                    { return setData(lnNum, WHILE, yytext); }
"for"                      { return setData(lnNum, FOR, yytext); }
"step"                     { return setData(lnNum, BY, yytext); }
"static"                   { return setData(lnNum, STATIC, yytext); }
"return"                   { return setData(lnNum, RETURN, yytext); }
"break"                    { return setData(lnNum, BREAK, yytext); }
"and"                      { return setData(lnNum, AND, yytext); }
"not"                      { return setData(lnNum, NOT, yytext); }
"end"                      { return setData(lnNum, END, yytext); }
"begin"                    { return setData(lnNum, BEG, yytext); }
"or"                       { return setData(lnNum, OR, yytext); }
".."                       { return setData(lnNum, TO, yytext); }
"<="                       { return setData(lnNum, ASGN, yytext); }
"+="                       { return setData(lnNum, ADDASS, yytext); }
"-="                       { return setData(lnNum, SUBASS, yytext); }
"*="                       { return setData(lnNum, MULASS, yytext); }
"/="                       { return setData(lnNum, DIVASS, yytext); }
"<"                        { return setData(lnNum, LESS, yytext); }
">"                        { return setData(lnNum, GREAT, yytext); }
"!>"                       { return setData(lnNum, LEQ, yytext); }
"!<"                       { return setData(lnNum, GEQ, yytext); }
"="                        { return setData(lnNum, EQL, yytext); }
"!="                       { return setData(lnNum, NEQ, yytext); }
"+"                        { return setData(lnNum, ADD, yytext); }
"-"                        { return setData(lnNum, SUB, yytext); }
"*"                        { return setData(lnNum, MUL, yytext); }
"/"                        { return setData(lnNum, DIV, yytext); }
"%"                        { return setData(lnNum, MOD, yytext); }
"?"                        { return setData(lnNum, QUES, yytext); }
"["                        { return setData(lnNum, OBRK, yytext); }
"++"                       { return setData(lnNum, INC, yytext); }
"--"                       { return setData(lnNum, DEC, yytext); }
[\],;:}\(\)]               { return yytext[0]; }
{true}|{false}             { return setData(lnNum, BOOLCONST, yytext); }
{identifier}               { return setData(lnNum, ID, yytext); }
{number}                   { return setData(lnNum, NUMCONST, yytext); }
\"(?:[^\\"\n]*|\\.)*\"     { return setData(lnNum, STRINGCONST, yytext); }      
'(?:[^\\'\n]*|\\.)*'       { return setData(lnNum, CHARCONST, yytext); }
{newline}                  { lnNum++; }
{whitespace}               { /* Ignored */ }
{comment}                  { /* Ignored */ }
.                          { printf("ERROR(%d): Invalid or misplaced input character: \'%s\'\n", lnNum, yytext); }
%%

int setData(int lnNum, int tknClass, char *yytext)
{
   yylval.tokenData = new struct TokenData;
   yylval.tokenData->tknClass = tknClass;
   yylval.tokenData->lineNum = lnNum;
   yylval.tokenData->tknStr = strdup(yytext);

   if( tknClass == NUMCONST )
   {
      yylval.tokenData->nVal = atoi(yytext);
   }
   if( tknClass == STRINGCONST )
   {
      if( strlen(yytext) == 2 ) // ""
      {
         yylval.tokenData->strVal = yytext;
      }
      else {
         for( int i = 1; i < strlen(yytext) - 1; i++ )
         {
            if( yytext[i] == '\\' ) {
               int escSlash = i;
               switch(yytext[i+1])
               {
                  case 'f':
                     yytext = workEscChar(yytext, escSlash, '\f');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  case 'n':
                     yytext = workEscChar(yytext, escSlash, '\n');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  case 'r':
                     yytext = workEscChar(yytext, escSlash, '\r');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  case 't':
                     yytext = workEscChar(yytext, escSlash, '\t');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  case 'v':
                     yytext = workEscChar(yytext, escSlash, '\v');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  case '\\':
                     yytext = workEscChar(yytext, escSlash, '\\');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  case '\'':
                     yytext = workEscChar(yytext, escSlash, '\'');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  case '\"':
                     yytext = workEscChar(yytext, escSlash, '\"');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  case '\?':
                     yytext = workEscChar(yytext, escSlash, '\?');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  case '\0':
                     yytext = workEscChar(yytext, escSlash, '\0');
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
                  default:
                     yytext = workEscChar(yytext, escSlash, -1);
                     yylval.tokenData->strVal = strdup(yytext);
                     break;
               }
            }
         }
      }
      yylval.tokenData->strVal = yytext;
   }
   if( tknClass == CHARCONST ) 
   {
      if( strlen(yytext) == 2 ) // ''
      {
         yylval.tokenData->strVal = yytext;
      }
      else if( yytext[1] == '\\' )
      {
         switch(yytext[2])
         {
            case '^':
               yylval.tokenData->cVal = '^';
               break;
            case 'x':
               yylval.tokenData->cVal = 'x';
               break;            
            case '@':
               yylval.tokenData->cVal = '@';
               break;
            case 'd':
               yylval.tokenData->cVal = 'd';
               break;
            case 'f':
               yylval.tokenData->cVal = '\f';
               break;
            case 'n':
               yylval.tokenData->cVal = '\n';
               break;
            case 'r':
               yylval.tokenData->cVal = '\r';
               break;
            case 't':
               yylval.tokenData->cVal = 't';
               break;
            case 'v':
               yylval.tokenData->cVal = '\v';
               break;
            case '\\':
               yylval.tokenData->cVal = '\\';
               break;
            case '\'':
               yylval.tokenData->cVal = '\'';
               break;
            case '\"':
               yylval.tokenData->cVal = '\"';
               break;
            case '\?':
               yylval.tokenData->cVal = '\?';
               break;
            case '\0':
               yylval.tokenData->cVal = '\0';
               break;
            default:
               yylval.tokenData->cVal = (char)NULL;
               break;
         } 
      }
      else if( strlen(yytext) > 3 && yytext[1] != '\\' ) 
      {
         printf("WARNING(%d): character is %ld characters long and not a single character: \'%s\'. The first char will be used.\n", yylval.tokenData->lineNum, strlen(yytext) - 2, yytext);
         yylval.tokenData->cVal = yytext[1];
      }
      else {
         yylval.tokenData->cVal = yytext[1];
      }
   }
   if( tknClass == BOOLCONST )
   {
      yylval.tokenData->strVal = strdup(yytext);
      if( strcmp(yytext, "true") == 0 || strcmp(yytext, "True") == 0 )
      {
         yylval.tokenData->nVal = 1;
      }
      else {
         yylval.tokenData->nVal = 0;
      }
   }
   return tknClass;
}

char* workEscChar(char* yytext, int escSlash, char escChar)
{
   int escCharIndex = escSlash + 1;
   int shiftIndex = escCharIndex;
   if( escChar == -1 )
   {
      while( yytext[escSlash] != '\0' )
      {
         yytext[escSlash] = yytext[escSlash + 1];
         escSlash++;
      }      
   }
   else {
      yytext[escSlash] = escChar;
      while( yytext[shiftIndex] != '\0' )
      {
         yytext[shiftIndex] = yytext[shiftIndex + 1];
         shiftIndex++;
      }
   }
   return yytext;
}