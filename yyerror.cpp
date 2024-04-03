#include "yyerror.hpp"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>

// // // // // // // // // // // // // // // // // // // // 
//
// Error message printing
//
// Must make messages look nice.  For example:
// msg = "syntax error, unexpected ',', expecting BOOL or CHAR or INT or ID."
// becomes (xx marks important data):
//  0 syntax
//  1 error,
//  2 unexpected
//  3 ',',    xx
//  4 expecting
//  5 BOOL    xx
//  6 or
//  7 CHAR    xx
//  8 or
//  9 INT     xx
// 10 or
// 11 ID.     xx

// assumes a string with breakchar separating each element.
// breakchars will be replaced by null chars: '\0'
// array of pointers to strings is then returned in
// the array strs which must be allocated by the user!
// the number of strings found is returned as a value of
// the function.  This number is always at least 1.
// The array is terminated by a NULL so there must be
// enough room for all the string pointers plus one for the
// sentinal marker.
static int split(char *s, char *strs[], char breakchar)
{
    int num;
    
    strs[0] = s;
    num = 1;
    for (char *p = s; *p; p++) {
        if (*p==breakchar) {
            strs[num++] = p+1;
            *p = '\0';
        }
    }
    strs[num] = NULL;
    
    return num;
}

// trim off the last character
static void trim(char *s)
{
    s[strlen(s)-1] = '\0';
}
// map from string to char * for storing nice translation of
// internal names for tokens.  Preserves (char *) used by
// bison.
static std::map<std::string , char *> niceTokenNameMap;    // use an ordered map (not as fast as unordered)

// WARNING: this routine must be called to initialize mapping of
// (strings returned as error message) --> (human readable strings)
//
void initErrorProcessing() {
    niceTokenNameMap["CHAR"] = (char *)"\"char\"";
    niceTokenNameMap["BOOL"] = (char *)"\"bool\"";
    niceTokenNameMap["INT"] = (char *)"\"int\"";
    niceTokenNameMap["IF"] = (char *)"\"if\"";
    niceTokenNameMap["THEN"] = (char *)"\"then\"";
    niceTokenNameMap["DO"] = (char *)"\"do\"";
    niceTokenNameMap["ELSE"] = (char *)"\"else\"";
    niceTokenNameMap["WHILE"] = (char *)"\"while\"";
    niceTokenNameMap["FOR"] = (char *)"\"for\"";
    niceTokenNameMap["BY"] = (char *)"\"step\"";
    niceTokenNameMap["STATIC"] = (char *)"\"static\"";
    niceTokenNameMap["RETURN"] = (char *)"\"return\"";
    niceTokenNameMap["BREAK"] = (char *)"\"break\"";
    niceTokenNameMap["AND"] = (char *)"\"and\"";
    niceTokenNameMap["NOT"] = (char *)"\"not\"";
    niceTokenNameMap["BEG"] = (char *)"\"begin\"";
    niceTokenNameMap["END"] = (char *)"\"end\"";
    niceTokenNameMap["OR"] = (char *)"\"or\"";
    niceTokenNameMap["TO"] = (char *)"\"..\"";
    niceTokenNameMap["ASGN"] = (char *)"\"<=\"";
    niceTokenNameMap["ADDASS"] = (char *)"\"+=\"";
    niceTokenNameMap["SUBASS"] = (char *)"\"-=\"";
    niceTokenNameMap["MULASS"] = (char *)"\"*=\"";
    niceTokenNameMap["DIVASS"] = (char *)"\"/=\"";
    niceTokenNameMap["LESS"] = (char *)"\'<\'";
    niceTokenNameMap["GREAT"] = (char *)"\'>\'";
    niceTokenNameMap["LEQ"] = (char *)"\"!>\"";
    niceTokenNameMap["GEQ"] = (char *)"\"!<\"";
    niceTokenNameMap["EQL"] = (char *)"\'=\'";
    niceTokenNameMap["NEQ"] = (char *)"\"!=\"";
    niceTokenNameMap["ADD"] = (char *)"\'+\'";
    niceTokenNameMap["SUB"] = (char *)"\'-\'";
    niceTokenNameMap["MUL"] = (char *)"\'*\'";
    niceTokenNameMap["DIV"] = (char *)"\'/\'";
    niceTokenNameMap["MOD"] = (char *)"\'%\'";
    niceTokenNameMap["QUES"] = (char *)"\'?\'";
    niceTokenNameMap["OBRK"] = (char *)"\'[\'";
    niceTokenNameMap["INC"] = (char *)"\"++\"";
    niceTokenNameMap["DEC"] = (char *)"\"--\"";
    niceTokenNameMap["SIZEOF"] = (char *)"\'*\'";
    niceTokenNameMap["CHSIGN"] = (char *)"-";
    niceTokenNameMap["BOOLCONST"] = (char *)"Boolean constant";
    niceTokenNameMap["ID"] = (char *)"identifier";
    niceTokenNameMap["NUMCONST"] = (char *)"numeric constant";
    niceTokenNameMap["STRINGCONST"] = (char *)"string constant";
    niceTokenNameMap["CHARCONST"] = (char *)"character constant";
    niceTokenNameMap["$end"] = (char *)"end of input";
    niceTokenNameMap["$undefined"] = (char *)"undefined";
}

// looks in pretty printed words for tokens that are
// not already in single quotes.  It uses the niceTokenNameMap table.
static char *niceTokenStr(char *tokenName ) {
    if (tokenName[0] == '\'') return tokenName;
    if (niceTokenNameMap.find(tokenName) == niceTokenNameMap.end()) {
        printf("ERROR(SYSTEM) (%d): niceTokenStr fails to find string '%s'\n", lnNum, tokenName); 
        fflush(stdout);
        exit(1);
    }
    return niceTokenNameMap[tokenName];
}

// Is this a message that we need to elaborate with the current parsed token?
// This elaboration is some what of a crap shoot since the token could
// be already overwritten with a look ahead token.   But probably not.
static bool elaborate(char *s)
{
    return (strstr(s, "constant") || strstr(s, "identifier"));
}

// A tiny sort routine for SMALL NUMBERS of
// of char * elements.  num is the total length
// of the array but only every step elements will
// be sorted.  The "up" flag is direction of sort.
// For example:
//    tinySort(str, i, 2, direction);      // sorts even number elements in array
//    tinySort(str+1, i-1, 2, direction);  // sorts odd number elements in array
//    tinySort(str, i, 1, direction);      // sorts all elements in array
//
static void tinySort(char *base[], int num, int step, bool up)
{
    for (int i=step; i<num; i+=step) {
        for (int j=0; j<i; j+=step) {
            if (up ^ (strcmp(base[i], base[j])>0)) {
                char *tmp;
                tmp = base[i]; base[i] = base[j]; base[j] = tmp;
            }
        }
    }
}

// This is the yyerror called by the bison parser for errors.
// It only does errors and not warnings.   
void yyerror(const char *msg)
{
    /*printf("[YYERROR] (%d) %s\n", lnNum, msg);
    fflush(stdout);
    errs += 1;*/
    char *space;
    char *strs[100];
    int numstrs;

    // make a copy of msg string
    space = strdup(msg);

    // split out components
    numstrs = split(space, strs, ' ');
    if (numstrs>4) trim(strs[3]);

    // translate components
    for (int i=3; i<numstrs; i+=2) {
        strs[i] = niceTokenStr(strs[i]);
    }

    // print components
    printf("ERROR(%d): Syntax error, unexpected %s", lnNum, strs[3]);
    if (elaborate(strs[3])) {
        if (lastToken->tknStr[0] == '\'' || lastToken->tknStr[0] == '"') printf(" %s", lastToken->tknStr); 
        else printf(" \"%s\"", lastToken->tknStr);
    }

    if (numstrs>4) printf(",");

    // print sorted list of expected
    tinySort(strs+5, numstrs-5, 2, true); 
    for (int i=4; i<numstrs; i++) {
        printf(" %s", strs[i]);
    }
    printf(".\n");
    fflush(stdout);   // force a dump of the error

    errs++;      // count the number of errors

    free(space);
}