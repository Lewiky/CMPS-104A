// Matthew Tan
// mxtan
// cs104a
// asg3: main.cpp

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
FILE* astFile;
FILE* tokFile;
FILE* strFile;

void buildAsTree (const char* filename) {
    cpp_command = cpp_name + " " + filename;
    yyin = popen (cpp_command.c_str(), "r");
    if (yyin == nullptr) {
        syserrprintf (cpp_command.c_str());
        exit (exec::exit_status);
    }
    else {
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

void buildAstTokStrFileName(const char* filename)
{
   string fn = string(filename);
   fn = fn.substr(0, fn.find_last_of('.'));
   //printf("buildTokStrFileName: open tokFile\n");
   astFile = fopen((fn + ".ast").c_str(), "w");
   tokFile = fopen((fn + ".tok").c_str(), "w");
   strFile = fopen((fn + ".str").c_str(), "w");
   //printf("done\n");
}

int main (int argc, char** argv) {
    const char* fn = scan_opts (argc, argv);
    buildAstTokStrFileName(fn);
    //printf("main: buildTokStrFileName() called\n");
    buildAsTree(fn);
    //printf("main: buildAsTree() called\n");
    int parse_rc = yyparse();
    //printf("main: yyparse() called\n");
    cpp_pclose();
    //printf("main: cpp_pclose() called\n");

#ifdef debug_on
    //printf("main: calling yylex_destroy\n");
#endif
    yylex_destroy();
    //printf("main: yylex_destroy() called\n");
    stringset::dump(strFile);
    //printf("main: dump() called\n");

    if (parse_rc) {
        errprintf ("parse failed (%d) \n", parse_rc);
    } else {
        astree::print (astFile, yyparse_astree, 0);
#ifdef debug_on
        //printf("main: deleting parser::root\n");
#endif

        delete parser::root;
    }

   return exec::exit_status;
}

