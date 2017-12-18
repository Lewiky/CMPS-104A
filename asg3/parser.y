// Matthew Tan
// mxtan
// cs104a
// asg3: parser.y

%{

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "lyutils.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT

%token TOK_RETURNVOID TOK_PARAM TOK_PROTOTYPE TOK_DECLID
%token TOK_NEWSTRING TOK_VARDECL TOK_INDEX TOK_FUNCTION

%right  TOK_IFELSE TOK_IF TOK_ELSE
%right  '='
%left   TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left   '+' '-'
%left   '*' '/' '%'
%right  TOK_POS TOK_NEG '!' TOK_NEW

%start  start

%%

start   : program               { yyparse_astree = $1; }
        ;

program : program structdef     { $$ = $1->adopt ($2); }
        | program function      { $$ = $1->adopt($2); }
        | program statement     { $$ = $1->adopt ($2); }  
        | program error '}'     { $$ = $1; }  
        | program error ';'     { $$ = $1; }  
        |                       { $$ = parser::new_parseroot(); }
        ;

structdef : TOK_STRUCT TOK_IDENT structcontents '}' { 
              destroy($4); 
              $2 = $2->change_sym($2, TOK_TYPEID); 
              $$ = $1->adopt($2, $3); }
          | TOK_STRUCT TOK_IDENT '{' '}' {
                destroy($3, $4);
                $2 = $2->change_sym($2, TOK_TYPEID);
                $$ = $1->adopt($2); }
          ;

structcontents : '{' fielddecl ';' { destroy($3); 
                  $$ = $1->adopt($2); }
               | structcontents fielddecl ';' { destroy($3); 
                  $$ = $1->adopt($2); }
               ;

fielddecl   : basetype TOK_IDENT { $2 = $2->change_sym($2, TOK_FIELD); 
                  $$ = $1->adopt($2); }
            | basetype TOK_ARRAY TOK_IDENT {
                $3 = $3->change_sym($3, TOK_FIELD);
                $$ = $2->adopt($1, $3); }
            ;

basetype : TOK_VOID        { $$ = $1; }
         | TOK_BOOL        { $$ = $1; }
         | TOK_CHAR        { $$ = $1; }
         | TOK_INT         { $$ = $1; }
         | TOK_STRING      { $$ = $1; }
         | TOK_TYPEID      { $$ = $1; }
         | TOK_IDENT       { $$ = $1->change_sym($1, TOK_TYPEID); }
         ;   

function : identdecl '(' ')' block { destroy($3);
                $2 = $2->change_sym($2, TOK_PARAM);
                $$ = new astree(TOK_FUNCTION, $1->lloc, "");
                $$ = $$->adopt($1, $2); $$ = $$->adopt($4); }
         | identdecl param ')' block { destroy($3); 
                $$ = new astree(TOK_FUNCTION, $1->lloc, ""); 
                $$ = $$->adopt($1, $2); $$ = $$->adopt($4); }
         | identdecl '(' ')' ';' { destroy($3, $4);
                $2 = $2->change_sym($2, TOK_PARAM);
                $$ = new astree(TOK_PROTOTYPE, $1->lloc, "");
                $$ = $$->adopt($1, $2); }
         | identdecl param ')' ';' { destroy($3, $4);
                $$ = new astree(TOK_PROTOTYPE, $1->lloc, ""); 
                $$ = $$->adopt($1, $2); }
         ;

param    : '(' identdecl { $1 = $1->change_sym($1, TOK_PARAM); 
                 $$ = $1->adopt($2); }
         | param ',' identdecl { destroy($2); $$ = $1->adopt($3); }
         ;

identdecl : basetype TOK_IDENT { $2 = $2->change_sym($2, TOK_DECLID);
                  $$ = $1->adopt($2); }
          | basetype TOK_ARRAY TOK_IDENT { 
                  $3 = $3->change_sym($3, TOK_DECLID);
                  $$ = $2->adopt($1, $3); }
          ;

block     :  body '}' { destroy($2); 
                  $$ = $1->change_sym($1, TOK_BLOCK); }
          | '{' '}' { 
                  destroy($2);   $$ = $1->change_sym($1, TOK_BLOCK); }
          ;

body      : '{' statement { $1 = $1->change_sym($1, TOK_BLOCK);
                  $$ = $1->adopt($2); }
          | body statement { $$ = $1->adopt($2); }
          ;

statement : block    { $$ = $1; }
          | vardecl  { $$ = $1; }
          | while    { $$ = $1; }
          | ifelse   { $$ = $1; }
          | return   { $$ = $1; }
          | expr ';' { destroy($2); $$ = $1; }
          | ';'      { $$ = $1; }
          ;

vardecl   : identdecl '=' expr ';' { destroy($4); 
                  $2 = $2->change_sym($2, TOK_VARDECL);
                  $$ = $2->adopt($1, $3); }
          ;

while     : TOK_WHILE '(' expr ')' statement { destroy($2, $4); 
                  $$ = $1->adopt($3, $5); }
          ;

ifelse    : TOK_IF '(' expr ')' statement TOK_ELSE statement { 
                  destroy($2, $4);
                  $1->change_sym($1, TOK_IFELSE);
                  $$ = $1->adopt($3, $5);
                  $$ = $$->adopt($7); }
          | TOK_IF '(' expr ')' statement { 
                  destroy($2, $4); $$ = $1->adopt($3, $5); }
          ;

return    : TOK_RETURN ';' { destroy($2); 
          $$ = $1->change_sym($1, TOK_RETURNVOID); }
          | TOK_RETURN expr ';' { destroy($3); $$ = $1->adopt($2); }
          ;

expr      : expr binop expr { $$ = $2->adopt($1, $3); }
          | unop expr       { $$ = $$->adopt($2); }
          | allocator       { $$ = $1; }
          | call            { $$ = $1; }
          | '(' expr ')'    {  destroy($1, $3); $$ = $2; }
          | variable        { $$ = $1; }
          | constant        { $$ = $1; }
          ;

binop     : TOK_EQ          { $$ = $1; }
          | TOK_NE          { $$ = $1; }
          | TOK_LT          { $$ = $1; }
          | TOK_LE          { $$ = $1; }
          | TOK_GT          { $$ = $1; }
          | TOK_GE          { $$ = $1; }
          | '+'             { $$ = $1; }
          | '-'             { $$ = $1; }
          | '*'             { $$ = $1; }
          | '/'             { $$ = $1; }
          | '='             { $$ = $1; }
          ;

unop      : TOK_POS         { $$ = $1; }
          | TOK_NEG         { $$ = $1; }
          | '!'             { $$ = $1; }
          | TOK_NEW         { $$ = $1; }
          ;

allocator   : TOK_NEW TOK_IDENT '(' ')' { destroy($3, $4);
                    $2 = $2->change_sym($2, TOK_TYPEID);
                    $$ = $1->adopt($2); }
            | TOK_NEW TOK_STRING '(' expr ')' { destroy($3, $5); 
                    $1 = $1->change_sym($1, TOK_NEWSTRING);
                    $$ = $1->adopt($4); }
            | TOK_NEW basetype '[' expr ']' { destroy($3, $5);
                    $1 = $1->change_sym($1, TOK_NEWARRAY);
                    $$ = $1->adopt($2, $4); }
            ;

call        : cexprs ')' { destroy($2); $$ = $1; }
            | TOK_IDENT '(' ')' { destroy($3);
                    $2 = $2->change_sym($2, TOK_CALL);
                    $$ = $2->adopt($1); } ;

cexprs      : TOK_IDENT '(' expr { $2 = $2->change_sym($2, TOK_CALL);
                    $$ = $2->adopt($1, $3); }
            | cexprs ',' expr { 
                    destroy($2); $$ = $1->adopt($3); }
            ;

variable    : TOK_IDENT          { $$ = $1; }
            | expr '[' expr ']' { destroy($4); 
                    $2 = $2->change_sym($2, TOK_INDEX);
                    $$ = $2->adopt($1, $3); }
            | expr '.' TOK_IDENT { 
                    $3 = $3->change_sym($3, TOK_FIELD);
                    $$ = $2->adopt($1, $3);
            }
            ;

constant    : TOK_INTCON         { $$ = $1; }
            | TOK_CHARCON        { $$ = $1; }
            | TOK_STRINGCON      { $$ = $1; }
            | TOK_NULL           { $$ = $1; }
            ;

%%

const char *parser::get_tname(int symbol) {
    return yytname [YYTRANSLATE(symbol)];
}

astree *parser::new_parseroot (void) {
   yyparse_astree = new astree (TOK_ROOT,
       {lexer::lloc.filenr, 0, 0}, "");
   //printf("!!!parser.y: new_parseroot: 
   // lexer::lloc.filenr is: %lu\n", 
   //      lexer::filenames.size());
   return yyparse_astree;
}

const char *get_yytname(int symbol) {
    return yytname [YYTRANSLATE(symbol)];
}

/*
bool is_defined_token(int symbol) {
    return YYTRANSLATE(symbol) > YYUNDEFTOK;
}
*/
