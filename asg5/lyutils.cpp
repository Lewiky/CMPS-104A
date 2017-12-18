// Matthew Tan
// mxtan
// cmps104a
// asg5: lyutils.h

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "auxlib.h"
#include "lyutils.h"

using namespace std;

bool lexer::interactive = true;
location lexer::lloc = {0, 1, 0};
size_t lexer::last_yyleng = 0;
vector<string> lexer::filenames;
extern FILE* tokFile;

astree* parser::root = nullptr;

const string* lexer::filename (int filenr) {
    string* str = nullptr;
    try {
        str  = &lexer::filenames.at(filenr);
    }
    catch (...)
    {
    }
    return str;
}

void lexer::newfilename (const string& filename) {
    lexer::lloc.filenr = lexer::filenames.size();
    bool found = false;
    for (auto i: lexer::filenames) {
        if (i == filename) {
            found = true;
            break;
        }
    }
    if (!found) {
        lexer::filenames.push_back (filename);
    }
}

void lexer::advance() {
    if (not interactive) {
        if (lexer::lloc.offset == 0) {
            printf (";%2zd.%3zd: ",
                    lexer::lloc.filenr, lexer::lloc.linenr);
        }
        printf ("%s", yytext);
    }
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
    errllocprintf (lexer::lloc, "invalid source character (%s)\n",
                   buffer);
}

void lexer::badtoken (char* lexeme) {
    errllocprintf (lexer::lloc, "invalid token (%s)\n", lexeme);
}

void lexer::include() {
    size_t linenr;
    static char filename[0x1000];
    assert (sizeof filename > strlen (yytext));
    int scan_rc = sscanf (yytext, "# %zd \"%[^\"]\"", 
        &linenr, filename);

    fprintf(tokFile, "%s\n", yytext);

    if (scan_rc != 2) {
        errprintf ("%s: invalid directive, ignored\n", yytext);
    }else {
        if (yy_flex_debug) {
            fprintf (stderr, "--included # %zd \"%s\"\n",
                     linenr, filename);
        }
        lexer::lloc.linenr = linenr - 1;
        lexer::newfilename (filename);
    }
}

void yyerror (const char* message) {
    assert (not lexer::filenames.empty());
    errllocprintf (lexer::lloc, "%s\n", message);
}
