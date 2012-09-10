%{
#include "y.tab.h"
%}

DIGIT [0-9]

%%

-?{DIGIT}+  yylval.num = atoi( yytext ); return NUMBER;
[+*()]      return *yytext;

[ \t\n]+          /* eat up whitespace */
