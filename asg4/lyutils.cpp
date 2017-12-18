/* 
* Matthew Tan
* mxtan
* cs104a
* asg4: lyutils.cpp
*/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxlib.h"
#include "lyutils.h"

bool lexer::interactive = true;
location lexer::lloc = {0, 1, 0};
size_t lexer::last_yyleng = 0;
vector<string> lexer::filenames;

astree* parser::root = nullptr;
astree* yyparse_astree = nullptr;

const string* lexer::filename (int filenr) {
   return &lexer::filenames.at(filenr);
}

void lexer::newfilename (const string& filename) {
   lexer::lloc.filenr = lexer::filenames.size();
   //printf("in lyutils.cpp: newfilename: %lu\n", 
   //     lexer::lloc.filenr = lexer::filenames.size());
   //printf("lyutils.cpp: newfilename: %s\n", filename.c_str());

   //printf("pushing back: %s\n", filename.c_str());
   lexer::filenames.push_back (filename);
}

void lexer::advance() {
   if (not interactive) {
      if (lexer::lloc.offset == 0) {
         printf (";%2zd.%3zd: ",
                 lexer::lloc.filenr, lexer::lloc.linenr);
      }
      printf ("%s", yytext);
   }
   //printf("!!!in lyutils: advance: 
   // lexer::lloc.filenr is: %lu\n", 
   //    lexer::lloc.filenr);
   lexer::lloc.offset += last_yyleng;
   last_yyleng = yyleng;
}

void lexer::newline() {
   ++lexer::lloc.linenr;
   lexer::lloc.offset = 0;
}

void lexer::badchar (unsigned char bad) {
   char buffer[16];
   snprintf (buffer, sizeof buffer,
             isgraph (bad) ? "%c" : "\\%03o", bad);
   errprintf ("%s: %d: invalid source character (%s)\n",
              (filename(lexer::lloc.filenr))->c_str(), 
               lexer::lloc.linenr, buffer);
//   errllocprintf (lexer::lloc, "invalid source character (%s)\n",
//                  buffer);
}

void lexer::badtoken (char* lexeme) {
   errprintf ("%s: %d: invalid token (%s)\n",
              (filename(lexer::lloc.filenr))->c_str(), 
               lexer::lloc.linenr, lexeme);
//   errllocprintf (lexer::lloc, "invalid token (%s)\n", lexeme);
}

void lexer::include() {
   size_t linenr;
   static char filename[0x1000];
   assert (sizeof filename > strlen (yytext));
   int scan_rc = sscanf (yytext, "# %zd \"%[^\"]\"", &linenr, filename);
   if (scan_rc != 2) {
      errprintf ("%s: invalid directive, ignored\n", yytext);
   }else {
      fprintf (tokFile, "%s\n", yytext);
      lexer::lloc.linenr = linenr - 1;
      lexer::newfilename (filename);
   }
}

void yyerror (const char* message) {
   assert (not lexer::filenames.empty());
   errllocprintf (lexer::lloc, "%s\n", message);
}

