#ifndef SCANTYPE_H
#define SCANTYPE_H

struct TokenData {
   int tknClass;        // Token Class
   int lineNum;         // Line where token was found
   char *tknStr;        // String that was literally read
   char cVal;           // Stores any character value 
   int nVal;            // Stores any numeric value or Boolean
   char *strVal;        // Stores any string value
};

#endif