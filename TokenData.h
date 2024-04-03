#ifndef TOKENDATA_H
#define TOKENDATA_H

typedef struct TokenData {
   int tknClass;        // Token Class
   int lineNum;         // Line where token was found
   char *tknStr;        // String that was literally read
   char cVal;           // Stores any character value 
   int nVal;            // Stores any numeric value or Boolean
   char *strVal;        // Stores any string value
} TokenData;
#endif