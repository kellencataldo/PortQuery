 /* DEFINITIONS SECTION */

%option case-insensitive nodefault warn 8bit yylineno noyywrap
%option c++

%{
#include "sql.tab.h"
%}

%s BTWMODE

%%
 /* RULES SECTION */

ALL                     { return ALL; }
<BTWMODE>AND            { BEGIN INITIAL; return AND; }
AND                     { return ANDOP; }
ANY                     { return ANY; }
AS                      { return AS; }
ASC                     { return ASC; }
BETWEEN                 { BEGIN BTWMODE; return BETWEEN; }
BY                      { return BY; }
CASE                    { return CASE; }
CROSS                   { return CROSS; }
DESC                    { return DESC; }
DESCRIBE                { return DESCRIBE; } /* keep this? */
DISTINCT                { return DISTINCT; }
DISTINCTROW             { return DISTINCTROW; }
ELSE                    { return ELSE; }
ELSEIF                  { return ELSEIF; }
EXISTS                  { yylval.subtok = 0; return EXISTS; }
NOT[ \t\n]+EXISTS       { yylval.subtok = 1; return EXISTS; }
FROM                    { return FROM; }
GROUP                   { return GROUP; }
HAVING                  { return HAVING; }
IF                      { return IF; }
IN                      { return IN; }
INNER                   { return INNER; }
IS                      { return IS; }
JOIN                    { return JOIN; }
LEFT                    { return LEFT; }
LIKE                    { return LIKE; }
LIMIT                   { return LIMIT; }
MATCH                   { return MATCH; }
NATURAL                 { return NATURAL; }
NOT                     { return NOT; }
ON                      { return ON; }
OR                      { return OR; }
ORDER                   { return ORDER; }
OUTER                   { return OUTER; }
RIGHT                   { return RIGHT; }
SELECT                  { return SELECT; }
STRAIGHT_JOIN           { return STRAIGHT_JOIN; } /* keep this too? */
THEN                    { return THEN; } /* keep conditionals */
TO                      { return TO; }
UNION                   { return UNION; }
UNIQUE                  { return UNIQUE; }
USING                   { return USING; }
WHEN                    { return WHEN; }
WHERE                   { return WHERE; }
WITH                    { return WITH; }

%%

