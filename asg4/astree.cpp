/* 
* Matthew Tan
* mxtan
* cs104a
* asg4: astree.cpp
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
   parent = nullptr;
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
   parent = nullptr;
}

void astree::reorder_children () {
    vector<astree*> tmp;
    while (not children.empty()) {
        astree* child = children.back();
        tmp.push_back(child);
        children.pop_back();
    }
    for (uint i = 0; i < tmp.size(); i++) {
        children.push_back(tmp[i]);
    }
}

astree* astree::adopt (astree* child1, astree* child2, astree* child3){
    if (child1 != nullptr) {
        child1->parent = this;
        children.push_back (child1);
    }
    if (child2 != nullptr) {
        child2->parent = this;
        children.push_back (child2);
    }
    if (child3 != nullptr) {
        child3->parent = this;
        children.push_back (child3);
    }
/*
     printf("astree::adopt: ...done, tree contents\n");
     dump_tree(stderr);
*/
    return this;
}

astree* astree::adopt_sym (astree* child, int symbol_) {
#ifdef debug_on
    printf("astree::adopt_sym: symbol: %d\n", symbol_);
   astree::dump (stderr, child);
   printf ("\n");
#endif
    symbol = symbol_;
    printf("astree::adopt_sym: child: %s\n", child->lexinfo->c_str());
    return adopt (child);
}

astree* astree::change_sym (astree* tree, int symbol_){
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

string astree::attribute_string(attr_bitset attr, 
    int blocknr, string struct_name) {
    string strList;
    if (attr[ATTR_field] == 0) strList += "{"+to_string(blocknr)+"} ";
    if (attr[ATTR_field] == 1) {
        strList += "field {" + struct_name + "} ";
    } if (attr[ATTR_struct] == 1) {
        strList += "struct \"" + struct_name + "\" ";
    } if (attr[ATTR_int] == 1) {
        strList += "int ";
    } if (attr[ATTR_string] == 1) {
        strList += "string ";
    } if (attr[ATTR_variable] == 1) {
        strList += "variable ";
    } if (attr[ATTR_null] == 1) {
        strList += "null ";
    } if (attr[ATTR_function] == 1) {
        strList += "function ";
    } if (attr[ATTR_lval] == 1) {
        strList += "lval ";
    } if (attr[ATTR_param] == 1) {
        strList += "param ";
    } if (attr[ATTR_const] == 1) {
        strList += "const ";
    } if (attr[ATTR_vreg] == 1) {
        strList += "vreg ";
    } if (attr[ATTR_vaddr] == 1) {
        strList += "vaddr ";
    } if (attr[ATTR_void] == 1) {
        strList += "void ";
    } if (attr[ATTR_array] == 1) {
        strList += "array ";
    } if (attr[ATTR_prototype] == 1) {
        strList += "prototype ";
    }
    return strList;
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   for(int i = 0; i < depth; i++)
       fprintf(outfile, "%s", "|  ");

   string str = parser::get_tname(tree->symbol);
   int lastOccur = str.find_last_of('_');

   string tok = parser::get_tname (tree->symbol);
   if (lastOccur != -1)
   {
      tok = str.substr(lastOccur + 1, str.length());
   }
   
   string string_list = attribute_string(tree->attributes, 
       tree->blocknr, tree->struct_name);
   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd) %s\n",
            tok.c_str(), tree->lexinfo->c_str(),
            lexer::filenames.size() - 1, 
            tree->lloc.linenr, tree->lloc.offset, string_list.c_str());
   
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

void astree::transfer_param (astree* node) {
    for(uint i = 1; i < node->children.size(); ++i) {
        this->children.push_back(node->children[i]);
    }
    while(node->children.size() > 1) {
        node->children.erase(node->children.begin() + 1);
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
