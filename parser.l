%option noyywrap
%{
   #include <string.h>
   #include <bits/stdc++.h>
   #include "scanType.h"
   #include "parser.tab.h"

   int lnNum = 1;

   int setData(int lnNum, int tknClass, char *yytext);
   char* workEscChar(char* yytext, int escSlash, char escChar);
%}

digit [0-9]
number {digit}+
letter [a-zA-Z]
identifier {letter}+{digit}*
newline \n
whitespace [ \t]+
true ("true"|"True")
false ("false"|"False")
comment "##"(.*?)

%%                                                                      
"bool"                                                                  { printf("Line %d Token: BOOL\n", lnNum); }
"char"                                                                  { printf("Line %d Token: CHAR\n", lnNum); }
"int"                                                                   { printf("Line %d Token: INT\n", lnNum); }
"if"                                                                    { printf("Line %d Token: IF\n", lnNum); }
"then"                                                                  { printf("Line %d Token: THEN\n", lnNum); }
"do"                                                                    { printf("Line %d Token: DO\n", lnNum); }
"else"                                                                  { printf("Line %d Token: ELSE\n", lnNum); }
"while"                                                                 { printf("Line %d Token: WHILE\n", lnNum); }
"for"                                                                   { printf("Line %d Token: FOR\n", lnNum); }
"step"                                                                  { printf("Line %d Token: BY\n", lnNum); }
"static"                                                                { printf("Line %d Token: STATIC\n", lnNum); }
"return"                                                                { printf("Line %d Token: RETURN\n", lnNum); }
"break"                                                                 { printf("Line %d Token: BREAK\n", lnNum); }
"and"                                                                   { printf("Line %d Token: AND\n", lnNum); }
"not"                                                                   { printf("Line %d Token: NOT\n", lnNum); }
"end"                                                                   { printf("Line %d Token: END\n", lnNum); }
"begin"                                                                 { printf("Line %d Token: BEGIN\n", lnNum); }
"or"                                                                    { printf("Line %d Token: OR\n", lnNum); }
".."                                                                    { printf("Line %d Token: TO\n", lnNum); }
"<="                                                                    { printf("Line %d Token: ASGN\n", lnNum); }
"+="                                                                    { printf("Line %d Token: ADDASS\n", lnNum); }
"-="                                                                    { return setData(lnNum, OTHER, yytext); }
"*="                                                                    { return setData(lnNum, OTHER, yytext); }
"/="                                                                    { return setData(lnNum, OTHER, yytext); }
"<"                                                                     { return setData(lnNum, OTHER, yytext); }
">"                                                                     { return setData(lnNum, OTHER, yytext); }
"!>"                                                                    { printf("Line %d Token: LEQ\n", lnNum); }
"!<"                                                                    { printf("Line %d Token: GEQ\n", lnNum); }
"="                                                                     { return setData(lnNum, OTHER, yytext); }
"!="                                                                    { printf("Line %d Token: NEQ\n", lnNum); }
"+"                                                                     { return setData(lnNum, OTHER, yytext); }
"-"                                                                     { return setData(lnNum, OTHER, yytext); }
"*"                                                                     { return setData(lnNum, OTHER, yytext); }
"/"                                                                     { return setData(lnNum, OTHER, yytext); }
"%"                                                                     { return setData(lnNum, OTHER, yytext); }
"?"                                                                     { return setData(lnNum, OTHER, yytext); }
"["                                                                     { return setData(lnNum, OTHER, yytext); }
"]"                                                                     { return setData(lnNum, OTHER, yytext); }
"++"                                                                    { printf("Line %d Token: INC\n", lnNum); }
"--"                                                                    { printf("Line %d Token: DEC\n", lnNum); }
"("                                                                     { return setData(lnNum, OTHER, yytext); }
")"                                                                     { return setData(lnNum, OTHER, yytext); }
";"                                                                     { return setData(lnNum, OTHER, yytext); }
":"                                                                     { return setData(lnNum, OTHER, yytext); }
","                                                                     { return setData(lnNum, OTHER, yytext); }
{true}|{false}                                                          { return setData(lnNum, BOOLCONST, yytext); }
{identifier}                                                            { return setData(lnNum, ID, yytext); }
{number}                                                                { return setData(lnNum, NUMCONST, yytext); }
\"(?:[^\\"\n]*|\\.)*\"                                                  { return setData(lnNum, STRINGCONST, yytext); }      
'(?:[^\\'\n]*|\\.)*'                                                    { return setData(lnNum, CHARCONST, yytext); }
{newline}                                                               { lnNum++; }
{whitespace}                                                            { /* Ignored */ }
{comment}                                                               { /* Ignored */ }
.                                                                       { printf("ERROR(%d): Invalid or misplaced input character: \'%s\'. Character Ignored.\n", lnNum, yytext); }
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
   if( tknClass == OTHER )
   {
      yylval.tokenData->strVal = strdup(yytext);
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