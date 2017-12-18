/* 
* Matthew Tan
* mxtan
* cs104a
* asg3: astree.cpp
*/

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"

//#define debug_on
astree::astree (int symbol_, const location& lloc_, const char* info) {
   symbol = symbol_;
   lloc = lloc_;
   lexinfo = stringset::intern (info);

#ifdef debug_on
   printf("astree::astree: called.\n");
   dump_node(stdout);
   printf("\n");
#endif
   
   // vector defaults to empty -- no children
}

astree::~astree() {
#ifdef debug_on
    printf("astree::~astree: called.\n");
    printf("astree::~astree: num of children %lu\n", children.size());
#endif
 
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
#ifdef debug_on
      printf("astree::~astree: deleting child:\n");
      astree::dump (stderr, child);
   printf("\n");
#endif
      delete child;
      printf("astree::~astree: inside while, num of children %lu\n", 
           children.size());
   }

   if (yydebug) 
   {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2) {
#ifdef debug_on
   printf ("adopting child1\n");
   astree::dump (stderr, child1);
   printf ("\n");
   printf ("adopting child2\n");
   astree::dump (stderr, child2);
   printf ("\n");
#endif
   if (child1 != nullptr) children.push_back (child1);
   if (child2 != nullptr) children.push_back (child2);
   return this;
}

astree* astree::adopt_sym (astree* child, int symbol_) {
#ifdef debug_on
    printf("astree::adopt_sym: symbol: %d\n", symbol_);
   astree::dump (stderr, child);
   printf ("\n");
#endif
    symbol = symbol_;
    return adopt (child);
}

astree* astree::change_sym (astree* tree, int symbol_){
#ifdef debug_on
    printf("astree::change_sym: symbol: %d\n", symbol_);
    printf ("\n");
#endif
    tree->symbol = symbol_;
    return tree;
}

void astree::dump_node (FILE* outfile) {
   fprintf (outfile, "%p->{%s %zd.%zd.%zd \"%s\":",
            this, parser::get_tname (symbol),
            lloc.filenr, lloc.linenr, lloc.offset,
            lexinfo->c_str());
   for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }
}

void astree::dump_tree (FILE* outfile, int depth) {
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (nullptr);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_node (outfile);
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   for(int i = 0; i < depth; i++)
       fprintf(outfile, "%s", "|  ");

   string str = parser::get_tname(tree->symbol);
   //printf("in astree::print: str is: %s\n", str.c_str());
   int lastOccur = str.find_last_of('_');
   //printf("int astree::print: lastOccur is: %d\n", lastOccur);
   string tok = parser::get_tname (tree->symbol);
   if (lastOccur != -1)
   {
      tok = str.substr(lastOccur + 1, str.length());
      //printf("in astree::print: tok is: %s\n", tok.c_str());
   }
   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd)\n",
            tok.c_str(), tree->lexinfo->c_str(),
            lexer::filenames.size() - 1, 
            tree->lloc.linenr, tree->lloc.offset);
   
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

void destroy (astree* tree1, astree* tree2) {
#ifdef debug_on
    printf("destroy tree1\n");
    astree::dump (stderr, tree1);
    printf ("\n");

    printf("destroy tree2\n");
    astree::dump (stderr, tree2);
    printf ("\n");
#endif

   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
              buffer);
}
