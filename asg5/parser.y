// Matthew Tan
// mxtan
// cmps104a
// asg5: parser.y

%{
#include <cassert>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "lyutils.h"
#include <iostream>

using namespace std;

extern int exit_status;
%}

%debug
%defines
%error-verbose
%token-table
%verbose

%destructor { destroy ($$); } <>
%printer { astree::dump (yyoutput, $$); } <>

%initial-action {
    parser::root = new astree (ROOT, {0, 0, 0}, "<<ROOT>>");
 }

%token  ROOT TOK_IDENT NUMBER UNOP BINOP TOK_NEWSTRING TOK_INDEX
%token  TOK_IF TOK_ELSE TOK_IFELSE TOK_FIELD_LIST
%token  TOK_WHILE TOK_RET TOK_RETURNVOID
%token  TOK_INT TOK_STRING TOK_STRUCT TOK_VOID
%token  TOK_NEW TOK_NULL TOK_ARRAY TOK_VARDECL
%token  TOK_INT_CONST TOK_CHAR_CONST TOK_STR_CONST
%token  TOK_EQ TOK_NEQ TOK_LEQ TOK_GEQ
%token  TOK_BLOCK TOK_CALL TOK_DECLID TOK_FUNCTION TOK_PROTOTYPE
%token  TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token  TOK_ORD TOK_CHR TOK_ROOT TOK_PARAMLIST BAD_TOK TOK_EXC

%right  TOK_IF TOK_ELSE
%right  '=' 
%left   TOK_EQ TOK_NEQ '<' TOK_LEQ '>' TOK_GEQ 
%left   '+' '-'
%left   '*' '/' '%'
%right  POS NEG TOK_EXC TOK_NEW
%right  '^'
%left   '[' '.'

%nonassoc '('

%start  start

%%
start     : program                   { $$ = $1 = nullptr; }
          ;

program   : program structdef         { $$ = $1->adopt ($2); }
          | program function          { $$ = $1->adopt ($2); }
          | program stmt              { $$ = $1->adopt ($2); }
          | program error '}'         { destroy ($3); $$ = $1;  }
          | program error ';'         { destroy ($3); $$ = $1; }
          |                           { $$ = parser::root; }
          ;

structdef : TOK_STRUCT TOK_IDENT '{' '}'  
             { destroy ($3, $4);
               $2->change_sym (TOK_TYPEID);
               $$ = $1->adopt ($2); }
          | TOK_STRUCT TOK_IDENT '{' fields '}' 
             { destroy ($3, $5);
               $2->change_sym (TOK_TYPEID);
               $$ = $1->adopt ($2, $4); }

fields    : field ';' fields  
            { $3->change_sym (TOK_FIELD_LIST);
              destroy ($2); $3->adopt ($1); 
              $3->reorder_children ();
              $$ = $3; }
          | field ';' { $2->change_sym (TOK_FIELD_LIST);
            $$ = $2->adopt ($1); }
          ;

field     : basetype TOK_IDENT  
            { $2->change_sym (TOK_FIELD);
            $$ = $1->adopt ($2); }
          | basetype TOK_ARRAY TOK_IDENT  
            { $2->change_sym (TOK_ARRAY);
              $3->change_sym (TOK_FIELD);
              $$ = $2->adopt ($1, $3); }
          ;

basetype  : TOK_VOID     { $$ = $1; }
          | TOK_INT      { $$ = $1; }
          | TOK_STRING   { $$ = $1; }
          | TOK_IDENT    { $1->change_sym (TOK_TYPEID); 
                           $$ = $1; }
          ;

function  : identdecl '(' identdecls ')' block 
            { $4->change_sym (TOK_FUNCTION);
              $2->change_sym (TOK_PARAMLIST);
              $2->adopt ($3);
              $2->transfer_param ($3);
              $$ = $4->adopt ($1, $2, $5); }
          |  identdecl '(' identdecls ')' ';' 
             { destroy($5);
               $4->change_sym (TOK_PROTOTYPE);
               $2->change_sym (TOK_PARAMLIST);
               $2->adopt ($3);
               $2->transfer_param ($3);
               $$ = $4->adopt ($1, $2); }
         | identdecl '(' ')' ';'     { destroy ($4);
           $3->change_sym (TOK_PROTOTYPE);
           $2->change_sym (TOK_PARAMLIST);
           $$ = $3->adopt ($1, $2); }
         | identdecl '(' ')' block   
           { $3->change_sym(TOK_FUNCTION);
             $2->change_sym(TOK_PARAMLIST);
             $$ = $3->adopt ($1, $2, $4); }
         ;

identdecls: identdecls ',' identdecl  
            { destroy ($2);  $$ = $1->adopt ($3); }
          |  identdecl  { $$ = $1; }
          ;

identdecl : basetype TOK_ARRAY TOK_IDENT  
            { $3->change_sym (TOK_DECLID);
              $$ = $2->adopt ($1, $3); }
          | basetype TOK_IDENT        
            { $2->change_sym (TOK_DECLID);
              $$ = $1->adopt ($2); }
          ;

block     : stmts '}'
            { destroy ($2);
            $1->change_sym (TOK_BLOCK);
            $$ = $1; }
          | '{' '}'                   
            { destroy ($2);
           $1->change_sym (TOK_BLOCK);
           $$ = $1; }
          ;

stmts     : stmts stmt    
            { $$ = $1->adopt ($2); }
          | '{' stmt { $$ = $1->adopt ($2); }
          ;

stmt      : block     { $$ = $1; }
          | vardecl   { $$ = $1; }
          | while     { $$ = $1; }
          | ifelse    { $$ = $1; }
          | return    { $$ = $1; }
          | expr ';'  { destroy ($2); $$ = $1; }
          | ';'       { $$ = $1; }
          ;

vardecl   : identdecl '=' expr ';'    { destroy ($4);
            $2->change_sym (TOK_VARDECL);
            $$ = $2->adopt ($1, $3); }
          ;

while     : TOK_WHILE '(' expr ')' stmt        
            { destroy ($2, $4);
            $$ = $1->adopt($3, $5); }
          ;

ifelse    : TOK_IF '(' expr ')' stmt %prec TOK_IF 
            { destroy ($2, $4);
            $$ =$1->adopt($3,$5); }
          | TOK_IF '(' expr ')' stmt TOK_ELSE stmt
            { destroy ($2, $4);
              destroy ($6);
              $1->change_sym (TOK_IFELSE);
              $$ = $1->adopt ($3, $5, $7); }
          ;

return    : TOK_RET ';' 
            { destroy ($2);
            $1->change_sym (TOK_RETURNVOID);
            $$ = $1; }
          | TOK_RET expr ';'          
           { destroy ($3);
           $$ = $1->adopt ($2); }
          ;

expr      : binop  { $$ = $1; }
          | unop                      { $$ = $1; }
          | alloc                     { $$ = $1; }
          | call                      { $$ = $1; }
          | variable                  { $$ = $1; }
          | constant                  { $$ = $1; }
          | '(' expr ')'              { destroy ($1, $3); 
                                        $$ = $2; }
          ;

binop     : expr '=' expr  
            { $$ = $2->adopt ($1, $3); }
          | expr '+' expr  
            { $$ = $2->adopt ($1, $3); }
          | expr '-' expr             
            { $$ = $2->adopt ($1, $3); }
          | expr '*' expr             
            { $$ = $2->adopt ($1, $3); }
          | expr '/' expr             
            { $$ = $2->adopt ($1, $3); }
          | expr '>' expr             
            { $$ = $2->adopt ($1, $3); }
          | expr '<' expr 
            { $$ = $2->adopt ($1, $3); }
          | expr '^' expr             
            { $$ = $2->adopt ($1, $3); }
          | expr TOK_LEQ expr         
            { $$ = $2->adopt ($1, $3); }
          | expr TOK_GEQ expr         
            { $$ = $2->adopt ($1, $3); }
          | expr TOK_EQ expr          
            { $$ = $2->adopt ($1, $3); }
          | expr TOK_NEQ expr         
            { $$ = $2->adopt ($1, $3); }

unop      : '+' expr %prec POS     
          { $$ = $1->sym ($2, TOK_POS); }
          | '-' expr %prec NEG        
            { $$ = $1->sym ($2, TOK_NEG); }
          | '!' expr %prec TOK_EXC    
            { $$ = $1->sym ($2, TOK_EXC); }


alloc     : TOK_NEW TOK_IDENT '(' ')'        
          { destroy ($3, $4);
            $2->change_sym (TOK_TYPEID);
            $$ = $1->adopt ($2); }
          | TOK_NEW TOK_STRING '(' expr ')' 
          { destroy ($2); destroy($3, $5);
              $1->change_sym (TOK_NEWSTRING);
              $$ = $1->adopt ($4); }
          | TOK_NEW basetype '[' expr ']' 
          { destroy ($3, $5);
              $1->change_sym (TOK_NEWARRAY);
              $$ = $1->adopt ($2, $4); }

call      : params ')' { destroy ($2);
          $1->change_sym (TOK_CALL);
          $$ = $1; }
          | TOK_IDENT '(' ')' { destroy ($3);
            $2->change_sym (TOK_CALL);
            $$ = $2->adopt ($1); }
          ;

params    : TOK_IDENT '(' expr
          { $$ = $2->adopt ($1, $3); }
          |  params ',' expr { destroy ($2);
             $$ = $1->adopt ($3); }

variable  : TOK_IDENT { $$ = $1; }
          | expr '[' expr ']' { destroy ($4);
            $2->change_sym (TOK_INDEX);
            $$ = $2->adopt ($1, $3); }
          | expr '.' TOK_IDENT { 
            $3->change_sym(TOK_FIELD);
            $$ = $2->adopt ($1, $3); }
          ;

constant  : TOK_STR_CONST             { $$ = $1; }
          | TOK_CHAR_CONST            { $$ = $1; }
          | TOK_INT_CONST             { $$ = $1; }
          | TOK_NULL                  { $$ = $1; }
          ;
%%

const char* parser::get_tname (int symbol) {
    return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
    return YYTRANSLATE (symbol) > YYUNDEFTOK;
}


