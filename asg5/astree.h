/* 
* Matthew Tan
* mxtan
* cs104a
* asg5: astree.h
*/

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>

using namespace std;

#include "auxlib.h"

enum {
    ATTR_void, ATTR_int, ATTR_null, ATTR_string,
    ATTR_struct, ATTR_array, ATTR_function, ATTR_variable,
    ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval, ATTR_const,
    ATTR_vreg, ATTR_vaddr, ATTR_prototype, ATTR_bitset_size,
};

using attr_bitset = bitset<ATTR_bitset_size>;

struct location {
    size_t filenr;
    size_t linenr;
    size_t offset;
};

struct astree {
    int symbol;             
    location lloc;          
    const string* lexinfo;  
    vector<astree*> children;
    astree* parent;
    attr_bitset attributes;
    string* struct_name = nullptr;
    size_t blocknr;

    astree (int symbol, const location&, const char* lexinfo);
    ~astree();
    astree* adopt (astree* child1, astree* child2 = nullptr, 
                   astree* child3 = nullptr);
    astree* sym (astree* child, int symbol);
    void change_sym (int token);
    void dump_node (FILE*);
    void dump_tree (FILE*, int depth = 0);
    void transfer_param (astree* node);
    void reorder_children ();

    static string attribute_string(attr_bitset attr, 
                                   int blocknr, 
                                   string* struct_name);

    static void dump (FILE* outfile, astree* tree);
    static void print (FILE* outfile, astree* tree, int depth = 0);
};

extern astree* asroot;
astree* new_parseroot ();

void destroy (astree* tree1 = nullptr, astree* tree2 = nullptr);

void errllocprintf (const location&, const char* format, const char*);

 
#endif

