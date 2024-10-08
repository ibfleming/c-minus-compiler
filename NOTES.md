ASSIGNMENTS:


BOOLEAN:

not NOT, lhs, boolean types only, no arrays

UNARY:
-, CHSIGN, lhs, int type only, no arrays
*, SIZEOF, lhs, array only, no arrays
?, QUES, lhs, int type only, no arrays

CALL:
0 id
1 args/nullptr

[, ID_ARRAY: type is child 0 (lhs)
0 id -> array
1 index

processBinary():
and, AND
or, OR
=, OP, ARRAY
><, OP, ARRAY
<, OP, ARRAY
<=, OP, ARRAY
>, OP, ARRAY
>=, OP, ARRAY
:=, ASSIGNMENT, ARRAY
+=, ASSIGNMENT
-=, ASSIGNMENT
*=, ASSIGNMENT
/=, ASSIGNMENT
+, OP
-, OP
*, OP
/, OP
%, OP
[], ID_ARRAY

processUnary():
--, ASSIGNMENT
++, ASSIGNMENT
not, NOT
*, SIZEOF, ARRAY
-, CHSIGN
?, QUES