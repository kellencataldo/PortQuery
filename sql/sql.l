 /* DEFINITIONS SECTION */

%option case-insensitive nodefault warn 8bit yylineno noyywrap
%option c++

%{
#include "sql.tab.h"
%}

%s BTWMODE

%%
 /* RULES SECTION */

 /* tokens */

ALL                              { return ALL; }
<BTWMODE>AND                     { BEGIN INITIAL; return AND; }
AND                              { return ANDOP; }
ANY                              { return ANY; }
AS                               { return AS; }
ASC                              { return ASC; }
BETWEEN                          { BEGIN BTWMODE; return BETWEEN; }
BY                               { return BY; }
CASE                             { return CASE; }
COUNT                            { return FCOUNT; }
CROSS                            { return CROSS; }
DESC                             { return DESC; }
DESCRIBE                         { return DESCRIBE; } /* keep this? */
DISTINCT                         { return DISTINCT; }
DISTINCTROW                      { return DISTINCTROW; }
ELSE                             { return ELSE; }
ELSEIF                           { return ELSEIF; }
EXISTS                           { yylval.subtok = 0; return EXISTS; }
NOT[ \t\n]+EXISTS                { yylval.subtok = 1; return EXISTS; }
FROM                             { return FROM; }
GROUP                            { return GROUP; }
HAVING                           { return HAVING; }
IF                               { return IF; }
IN                               { return IN; }
INNER                            { return INNER; }
IS                               { return IS; }
JOIN                             { return JOIN; }
LEFT                             { return LEFT; }
LIKE                             { return LIKE; }
LIMIT                            { return LIMIT; }
MATCH                            { return MATCH; }
NATURAL                          { return NATURAL; }
NOT                              { return NOT; }
ON                               { return ON; }
OR                               { return OR; }
ORDER                            { return ORDER; }
OUTER                            { return OUTER; }
RIGHT                            { return RIGHT; }
SELECT                           { return SELECT; }
STRAIGHT_JOIN                    { return STRAIGHT_JOIN; } /* keep this too? */
THEN                             { return THEN; } /* keep conditionals */
TO                               { return TO; }
UNION                            { return UNION; }
UNIQUE                           { return UNIQUE; }
USING                            { return USING; }
WHEN                             { return WHEN; }
WHERE                            { return WHERE; }
WITH                             { return WITH; }


 /* numbers */ 

-?[0-9]+                         { yylval.intval = atoi(yytext); return INTNUM; } /* change this to strol? */

-?[0-9]+"."[0-9]*             |
-?"."[0-9]+                   |
-?[0-9]+E[-+]?[0-9]+          |
-?[0-9]+"."[0-9]*E[-+]?[0-9]+ |
-?"."[0-9]+E[-+]?[0-9]+          { yylval.floatval = atof(yytext);
                                   return APPROXNUM; }

 /* comparison */

"&&"                             { return ANDOP; }
"||"                             { return OR; }
"="                              { yylval.subtok = 4; return COMPARISON; }
"<=>"                            { yylval.subtok = 12; return COMPARISON; }
">="                             { yylval.subtok = 6; return COMPARISON; }
">"                              { yylval.subtok = 2; return COMPARISON; }
"<="                             { yylval.subtok = 5; return COMPARISON; }
"<"                              { yylval.subtok = 1; return COMPARISON; }
"!="                          |
"<>"                             { yylval.subtok = 3; return COMPARISON; }

[ \t\n]         /* whitespace */
.               { yyerror("mystery character '%c'", *yytext); }

%%

