/* 
* Matthew Tan
* mxtan
* cs104a
* asg3: scanner.l
*/

%{

#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance(); }

// #define debug_on
int yylval_token (int symbol) {
   yylval = new astree (symbol, lexer::lloc, yytext);

#ifdef debug_on
   fprintf(stderr, "%3lu %2lu.%03lu %4u  %-15s (%s)\n ", 
           lexer::lloc.filenr, lexer::lloc.linenr, 
           lexer::lloc.offset, symbol,  
           parser::get_tname(symbol), yytext);
   fprintf(stderr, "write to tokFile\n");
#endif

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

LETTER          [A-Za-z_]
DIGIT           [0-9]
NUMBER          {DIGIT}+
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
BAD_IDENT       ({NUMBER}+{IDENT})
CHAR            ('([^\\'\n]|\\[\\'"0nt])')
STRING          (\"([^\\"\n]|\\[\\'"0nt])*\")
BAD_STRING      (\"[^\n\"\\]*(.|\n))
ABAD_CHAR       ('(.*)|\n')
BAD_CHAR         ('[\n\'\\]'|'.[^\'])
%%

"#".*           { lexer::include(); }
[ \t]+          { }
\n              { lexer::newline(); }

"if"           { return yylval_token (TOK_IF);     }
"else"         { return yylval_token (TOK_ELSE);   }
"while"        { return yylval_token (TOK_WHILE);  }
"return"       { return yylval_token (TOK_RETURN); }
"int"          { return yylval_token (TOK_INT);    }
"string"       { return yylval_token (TOK_STRING); }
"struct"       { return yylval_token (TOK_STRUCT); }
"void"         { return yylval_token (TOK_VOID);   }
"new"          { return yylval_token (TOK_NEW);    }
"ord"          { return yylval_token (TOK_ORD);    }
"char"         { return yylval_token (TOK_CHAR);   }
"null"         { return yylval_token (TOK_NULL);   }
"chr"          { return yylval_token (TOK_CHR);    }

"+"            { return yylval_token ('+');  }
"-"            { return yylval_token ('-');  }
"*"            { return yylval_token ('*');  }
"/"            { return yylval_token ('/');  }
"%"            { return yylval_token ('%');  }
"!"            { return yylval_token ('!');  }
"="            { return yylval_token ('=');  }
","            { return yylval_token (',');  }
"."            { return yylval_token ('.');  }
";"            { return yylval_token (';');  }
"("            { return yylval_token ('(');  }
")"            { return yylval_token (')');  }
"["            { return yylval_token ('[');  }
"]"            { return yylval_token (']');  }
"{"            { return yylval_token ('{');  }
"}"            { return yylval_token ('}');  }
"=="           { return yylval_token (TOK_EQ); }
"!="           { return yylval_token (TOK_NE); }
"<"            { return yylval_token (TOK_LT); }
"<="           { return yylval_token (TOK_LE); }
">"            { return yylval_token (TOK_GT); }
">="           { return yylval_token (TOK_GE); }
"[]"           { return yylval_token (TOK_ARRAY);  }

{NUMBER}       { return yylval_token (TOK_INTCON); }
{IDENT}        { return yylval_token (TOK_IDENT); }
{BAD_IDENT}    { lexer::badtoken(yytext); }
{CHAR}         { return yylval_token (TOK_CHARCON); }
{STRING}       { return yylval_token (TOK_STRINGCON);}
{BAD_CHAR}     { lexer::badtoken(yytext); } 
{BAD_STRING}   { lexer::badtoken(yytext); }

.              { lexer::badchar (*yytext); }

%%

