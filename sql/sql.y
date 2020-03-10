/*****************
DEFINITIONS SECION 
******************/


%{
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void yyerror(char *s, ...);
void emit(char *s, ...);
%}


%union {
        int intval;
        double floatval;
        char *strval;
        int subtok;
}

