/* 
* Matthew Tan
* mxtan
* cs104a
* asg5: scanner.l
*/

%{

#include "lyutils.h"
#include "astree.h"
#include <iostream>
using namespace std;

#define YY_USER_ACTION  { lexer::advance(); }

extern FILE* tokFile;

int yylval_token (int symbol) {
   yylval = new astree (symbol, lexer::lloc, yytext);
   fprintf(tokFile, "%3lu %2lu.%03lu %4u  %-15s (%s)\n ", 
           lexer::lloc.filenr, lexer::lloc.linenr, 
           lexer::lloc.offset, symbol,  
           parser::get_tname(symbol), yytext);
   return symbol;
}

%}

%option 8bit
%option debug
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option warn
/*%option verbose*/


LETTER          [A-Za-z_]
DIGIT           [0-9]
MANTISSA        ({DIGIT}+\.?{DIGIT}*|\.{DIGIT}+)
EXPONENT        ([Ee][+-]?{DIGIT}+)
NUMBER          ({MANTISSA}{EXPONENT}?)
NOTNUMBER       ({MANTISSA}[Ee][+-]?)
IDENT           ({LETTER}({LETTER}|{DIGIT})*)

STRING          (\"([^\\"\n]|\\[\\'"0nt])*\")
CHAR            ('([^\\'\n]|\\[\\'"0nt])')
%%

"#".*           { lexer::include(); }
[ \t]+          { }
\n              { lexer::newline(); }


("if")            { return yylval_token (TOK_IF); }
("else")          { return yylval_token (TOK_ELSE); }
("while")         { return yylval_token (TOK_WHILE); }
("return")        { return yylval_token (TOK_RET); }
("int")           { return yylval_token (TOK_INT); }
("string")        { return yylval_token (TOK_STRING); }
("struct")        { return yylval_token (TOK_STRUCT); }
("void")          { return yylval_token (TOK_VOID); }
("new")           { return yylval_token (TOK_NEW); }
("null")          { return yylval_token (TOK_NULL); }

(<=)            { return yylval_token (TOK_LEQ); }
(>=)            { return yylval_token (TOK_GEQ); }
(==)            { return yylval_token (TOK_EQ); }
(!=)            { return yylval_token (TOK_NEQ); }

{STRING}        { return yylval_token (TOK_STR_CONST); }
{CHAR}          {return yylval_token (TOK_CHAR_CONST); }

(\[\])          { return yylval_token (TOK_ARRAY); }
{NUMBER}        { return yylval_token (TOK_INT_CONST); }
{IDENT}         { return yylval_token (TOK_IDENT); }
"="             { return yylval_token ('='); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"^"             { return yylval_token ('^'); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
";"             { return yylval_token (';'); }
"<"             { return yylval_token ('<'); }
">"             { return yylval_token ('>'); }
"."             { return yylval_token ('.'); }
","             { return yylval_token (','); }
"%"             { return yylval_token ('%'); }
"!"             { return yylval_token ('!'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }

{NOTNUMBER}     {  lexer::badtoken (yytext);
                  return yylval_token (NUMBER); }
.               { lexer::badchar (*yytext); }

%%
