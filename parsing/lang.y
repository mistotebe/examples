%{
#include <stdio.h>
extern FILE *yyin;
extern int errno;
int result;
%}

%token <num> NUMBER
%type <num> expr

/* The operators need to be listed in ascending precedence */
%left '+'
%left '*'

%union {
    int num;
}

%start expression

%%

expression: expr { result = $1; }
          ;

expr: expr '*' expr { $$ = $1 * $3; }
    | expr '+' expr { $$ = $1 + $3; }
    | '(' expr ')' { $$ = $2; }
    | NUMBER
    ;

%%

int main(int argc, char **argv)
{
    yyin = stdin;
    if (yyparse())
        return 1;

    printf("%d", result);
    return 0;
}

int yywrap() {
    return(1);
}

int yyerror(char *err) {
    fprintf(stderr, "%s\n",err);
}

