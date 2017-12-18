// Matthew Tan
// mxtan
// cs104a
// asg2: main.cpp

#include <string>
#include <vector>
using namespace std;

#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "stringset.h"

//#define debug_on

string cpp_name = "/usr/bin/cpp";
string cpp_command;
FILE* tokFile;
FILE* strFile;

void buildAsTree (const char* filename) {
    cpp_command = cpp_name + " " + filename;
    yyin = popen (cpp_command.c_str(), "r");
    if (yyin == nullptr) {
        syserrprintf (cpp_command.c_str());
        exit (exec::exit_status);
    }
    else 
    {
        int token = yylex();         
        while(token) {
#ifdef debug_on
            printf("!!!token value: %s\n", parser::get_tname(token));
            printf("value:%s\n", yytext);
            printf("loc: %lu.%lu.%lu\n", lexer::lloc.filenr,
                    lexer::lloc.linenr, lexer::lloc.offset );
#endif
            if(token == YYEOF) {
                return;
            }
            token = yylex();
#ifdef debug_on
            printf("token:%d\n", token);
#endif
        }
        lexer::newfilename (filename);
    }
}

void cpp_pclose() {
   int pclose_rc = pclose (yyin);
   eprint_status (cpp_command.c_str(), pclose_rc);
   if (pclose_rc != 0) exec::exit_status = EXIT_FAILURE;
}

const char* scan_opts (int argc, char** argv) 
{
   opterr = 0;
   yy_flex_debug = 0;
   yydebug = 0;
   lexer::interactive = isatty (fileno (stdin))
                    and isatty (fileno (stdout));
   for(;;) {
      int opt = getopt (argc, argv, "@lyD:");
      if (opt == EOF) break;
      switch (opt) {
         case '@': set_debugflags (optarg);   break;
         case 'l': yy_flex_debug = 1;         break;
         case 'y': yydebug = 1;               break;
         case 'D': cpp_name = cpp_name + " -D" + optarg; 
             printf("%s\n", cpp_name.c_str());
         break;
         default:  errprintf ("bad option (%c)\n", optopt); break;
      }
   }
   // set executable name
   exec::execname = argv[0];
   if (optind > argc) {
      errprintf ("Usage: %s [-ly] [filename]\n",
                 exec::execname.c_str());
      exit (exec::exit_status);
   }

   const char* filename = optind == argc ? "-" : argv[optind];
   string outFileName = filename;
   if (outFileName.substr( outFileName.find_last_of(".") + 1) != "oc"){
      errprintf ("Usage: %s [-ly] [filename]\n",
                 exec::execname.c_str());
      exit (exec::exit_status);
   }
   return filename;
}

void buildTokStrFileName(const char* filename)
{
   string fn = string(filename);
   fn = fn.substr(0, fn.find_last_of('.'));
   tokFile = fopen((fn + ".tok").c_str(), "w");
   strFile = fopen((fn + ".str").c_str(), "w");
}

int main (int argc, char** argv) {
   const char* fn = scan_opts (argc, argv);
   buildTokStrFileName(fn);
   buildAsTree(fn);
   stringset::dump(strFile);
   return exec::exit_status;
}

