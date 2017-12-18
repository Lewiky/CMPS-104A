/* 
* Matthew Tan
* mxtan
* cs104a
* asg4: symtable.h
*/

#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <string>
#include <vector>
#include <bitset>
#include <unordered_map>
#include <iostream>
#include "astree.h"

using namespace std;

struct symbol;

using symbol_table = unordered_map<string*, symbol*>;
using symbol_entry = symbol_table::value_type;


struct symbol {
    size_t filenr, linenr, offset;
    size_t blocknr = 0;
    attr_bitset attributes;
    vector<symbol*>* parameters = nullptr;
    symbol_table* fields;
    string struct_name = "";
};
bool    create_identifiers (FILE* outFile, astree* node);
void    handle_function(FILE* outFile, astree* node);
bool    handle_vardecl(FILE* outFile, astree* node);
bool    handle_struct(FILE* outFile, astree* node);
void    write_symbol_to_file (FILE* outFile, symbol* sym, string* name);
bool    is_valid_variable(string* name);
bool    is_return_type_match (symbol* sym1, symbol* sym2);
void    init_symbol (astree* node, symbol* sym);
symbol* new_symbol (astree* node);
symbol* init_function_attr (astree* node);
symbol* get_symbol (astree* node);
bool    type_check (astree* node);
bool    is_type_match (astree* node1, astree* node2,  
            string struct1, string struct2);

#endif
