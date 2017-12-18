// Matthew Tan
// mxtan
// cs104a
// asg4: symtable.cpp

#include "symtable.h"
#include "yyparse.h"

using namespace std;

symbol_table identifiers;
symbol_table struct_table;
vector<symbol_table*> symbol_stack;
vector<int> block_stack;
unordered_map<string, symbol_table*> field_table;
int next_block = 0;
bool block_incr = true;

//#define debug_on

void handle_function (FILE* outFile, astree* node) {
    string* name = const_cast<string*>( 
        node->children[0]->children[0]->lexinfo);
    if (node->children[0]->symbol == TOK_ARRAY) {
        name = const_cast<string*>(
            node->children[0]->children[1]->lexinfo);
    }
        
    if (identifiers.find(name) != identifiers.end()) {
        if (identifiers[name]->attributes[ATTR_function] == 1) {
            errprintf("%zd:%zd.%zd: error: ",
            "'%s' duplicate function defination\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return;
        }
    }
    if (symbol_stack.back() == nullptr) {
            symbol_stack.pop_back();
            symbol_stack.push_back(new symbol_table);
    }

    symbol* sym = init_function_attr(node);
    astree* child0 = node->children[0];

    write_symbol_to_file(outFile, sym, name);

    if (child0->symbol != TOK_ARRAY) {
        child0->children[0]->attributes = sym->attributes;
        child0->children[0]->struct_name = sym->struct_name;
    } else {
        child0->children[1]->attributes = sym->attributes;
        child0->children[1]->struct_name = sym->struct_name;
    }
    sym->blocknr = 0;
    block_stack.push_back(next_block++);

    sym->parameters = new vector<symbol*>;
    symbol_stack.push_back(new symbol_table);

    for(size_t i = 0; i < node->children[1]->children.size(); ++i) {
        block_incr = false;
        astree* tmp = node->children[1]->children[i];
        symbol* param;
        if (tmp->symbol == TOK_ARRAY)
            param = new_symbol(tmp->children[0]);
        else
            param = new_symbol(tmp);
        param->attributes[ATTR_param] = 1;
        sym->parameters->push_back(param);

        if (tmp->symbol != TOK_ARRAY) {
            tmp->children[0]->attributes = param->attributes;
            tmp->children[0]->blocknr = param->blocknr;
            tmp->children[0]->struct_name = param->struct_name;
        } else {
            param->attributes[ATTR_array] = 1;
            tmp->children[1]->attributes = param->attributes;
            tmp->children[1]->blocknr = param->blocknr;
            tmp->children[1]->struct_name = param->struct_name;
        }

        string* param_name;
        if (tmp->symbol == TOK_ARRAY)
            param_name = const_cast<string*>(tmp->children[1]->lexinfo);
        else
            param_name = const_cast<string*>(tmp->children[0]->lexinfo);

        symbol_entry param_entry(param_name, param);

        symbol_stack.back()->insert(param_entry);
        write_symbol_to_file(outFile, param, param_name);
    }
    if (block_incr == false) {
        next_block--;
        block_incr = true;
    }
    if (identifiers.find(name) != identifiers.end()) {
         if (identifiers[name]->attributes[ATTR_prototype] == 1) {
            if (identifiers[name]->parameters != sym->parameters) {
            errprintf("%zd:%zd.%zd: error: %s parameter mismatch\n", 
                      node->lloc.filenr, node->lloc.linenr, 
                      node->lloc.offset, node->lexinfo->c_str() );
            }
            if (is_return_type_match(identifiers[name], sym) == false) {
                errprintf("%zd:%zd.%zd: error: %s type mismatch\n", 
                      node->lloc.filenr, node->lloc.linenr, 
                      node->lloc.offset, node->lexinfo->c_str() );
            }
        }
    }

    symbol_entry entry(name, sym);
    symbol_stack.back()->insert(entry);
    identifiers.insert(entry);
    fprintf(outFile, "\n");
}

bool handle_vardecl (FILE* outFile, astree* node) {
    if (symbol_stack.back() == nullptr) {
        symbol_stack.back() = new symbol_table;
    }

    string* name = const_cast<string*>(
        node->children[0]->children[0]->lexinfo);
    if (node->children[0]->symbol == TOK_ARRAY)
        name = const_cast<string*>(
            node->children[0]->children[1]->lexinfo);

    symbol_table* back = symbol_stack.back();
    if (back != nullptr) {
        if (back->find(name) != back->end()) {
            if (!back->at(name)->attributes[ATTR_function]) {
                errprintf("%zd:%zd.%zd: note:",
                " '%s' previously declared here\n", 
                node->lloc.filenr, node->lloc.linenr, 
                node->lloc.offset, name->c_str() );
            }
        }
    }
    symbol* sym = new symbol; 
    if (node->children[1]->symbol == TOK_IDENT) {
        string* find_name = const_cast<string*>(
            node->children[1]->lexinfo);

        if (!is_valid_variable(find_name)) {
            errprintf("%zd:%zd.%zd: invalid assignmen: '%s'\n", 
                       node->lloc.filenr, node->lloc.linenr, 
                       node->lloc.offset, find_name->c_str() );
            return false;
        }
        symbol* assignee = get_symbol(node->children[1]);
        init_symbol(node->children[0], sym);
        sym->attributes = assignee->attributes;
        sym->struct_name = assignee->struct_name;
        sym->parameters = assignee->parameters;
        sym->fields = assignee->fields;
    }
    else {
        if (node->children[0]->symbol == TOK_ARRAY) {
            sym = new_symbol(node->children[0]->children[0]);
            sym->attributes[ATTR_array] = 1;
        }
            else
                sym = new_symbol(node->children[0]);
    }

    symbol_entry entry(name, sym);
    symbol_stack.back()->insert(entry);
    astree* child0 = node->children[0];
    if (child0->symbol == TOK_ARRAY) {
        child0->children[1]->attributes = sym->attributes;
        child0->children[1]->struct_name = sym->struct_name;
        child0->children[0]->attributes[ATTR_array] = 1;
    } else {
        child0->children[0]->attributes = sym->attributes;
        child0->children[0]->struct_name = sym->struct_name;
    }

    if (sym->blocknr == 0)
        fprintf(outFile, "\n ");   

    write_symbol_to_file(outFile, sym, name);
    return true;
}

bool handle_struct (FILE* outFile, astree* node) {
    string* name = const_cast<string*>(
        node->children[0]->lexinfo);
    symbol* sym = new symbol;
    sym->attributes[ATTR_struct] = 1;
    sym->struct_name = *name;
    init_symbol(node, sym);
    sym->blocknr = 0;
    node->attributes = sym->attributes;
    node->struct_name = sym->struct_name;
    symbol_table* tbl = new symbol_table;
    symbol_entry entry(name, sym);
    struct_table.insert(entry);
    write_symbol_to_file(outFile, sym, name);

    if (node->children.size() > 1) {
        for(size_t i = 0; 
           i < node->children[1]->children.size(); 
           ++i) {
            astree* tmp = 
                node->children[1]->children[i];
            if (tmp->symbol == TOK_TYPEID) {
                if (struct_table.find(name) == 
                   struct_table.end()) {
                    errprintf("%zd:%zd.%zd: error: %s is undefined\n", 
                             node->lloc.filenr, node->lloc.linenr, 
                             node->lloc.offset, 
                             node->lexinfo->c_str() );
                    return true;
                }
            }
            fprintf(outFile, "   ");

            string* field_name = const_cast<string*>(
                tmp->children[0]->lexinfo);
            if (tmp->symbol == TOK_ARRAY)
                field_name = const_cast<string*>(
                tmp->children[1]->lexinfo);

            symbol* sym = new symbol;
            if (tmp->symbol == TOK_ARRAY) {
                    init_symbol(tmp->children[0], sym);
                    sym->attributes[ATTR_array] = 1;
            }
            else {
                init_symbol(tmp, sym);
            }
            sym->attributes[ATTR_field] = 1;
            sym->struct_name = *(const_cast<string*>(name));
               
            sym->attributes[ATTR_field] = 1;
            sym->blocknr = 0;
            //printf("handle_struct: inserting 
            // field_name %s\n", field_name->c_str());
            symbol_entry field(field_name, sym);
            tbl->insert(field);
            
            write_symbol_to_file(outFile, sym, field_name);
            tmp->children[0]->attributes = sym->attributes;
            tmp->children[0]->struct_name = *name;
        }
    }
    pair <string, symbol_table*> str(*name, tbl);
    // printf("handle_struct: inserting name %s\n", name->c_str());
    field_table.insert(str);
    sym->fields = tbl;

    fprintf(outFile, "\n");
    return true;
}

bool handle_prototype (FILE* outFile, astree* node) {
    astree* child0 = node->children[0];
    string* name = const_cast<string*>(child0->children[0]->lexinfo);
    if (child0->symbol == TOK_ARRAY)
        name = const_cast<string*>(child0->children[1]->lexinfo);

    if (identifiers.find(name) != identifiers.end()) {
        if (identifiers[name]->attributes[ATTR_function] == 1) {
            errprintf("%zd:%zd.%zd: error: %s duplicate defination\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return true;
        }
    }

    if (symbol_stack.back() == nullptr) {
        symbol_stack.pop_back();
        symbol_stack.push_back(new symbol_table);
    }

    symbol* sym = init_function_attr(node);
    sym->attributes[ATTR_prototype] = 1;

    block_stack.push_back(next_block++);

    sym->parameters = new vector<symbol*>;
    if (child0->symbol == TOK_ARRAY) {
        sym->attributes[ATTR_array] = 1;
        child0->children[1]->attributes = sym->attributes;
        child0->children[0]->attributes[ATTR_array] = 1;
    }
    else {
        child0->children[0]->attributes = sym->attributes;
    }

    symbol_stack.push_back(new symbol_table);
    astree* t1 = node->children[1];
    for(size_t i = 0; i < t1->children.size(); ++i) {
        astree* tmp = node->children[1]->children[i];
        symbol* param = new_symbol(t1->children[i]);
        param->attributes[ATTR_param] = 1;
        sym->parameters->push_back(param);
        tmp->children[0]->attributes = param->attributes;
        tmp->children[0]->blocknr = param->blocknr;
        tmp->children[0]->struct_name = param->struct_name;

        string* param_name = const_cast<string*>(
            tmp->children[0]->lexinfo);

        symbol_entry param_entry(param_name, param);

        symbol_stack.back()->insert(param_entry);
        write_symbol_to_file(outFile, param, param_name);
    }
    symbol_entry entry(name, sym);

    symbol_stack.back()->insert(entry);
    identifiers.insert(entry);
    return true;
}

bool create_identifiers (FILE* outFile, astree* node) {
    if (node != nullptr && node->symbol == TOK_ROOT)
    {
        symbol_stack.push_back(new symbol_table);
        block_stack.push_back(next_block++);
    }    

    node->blocknr = block_stack.back();
    switch(node->symbol) {
    case TOK_BLOCK: {
        block_incr = true;       
        if (block_incr) {
        symbol_stack.push_back(nullptr);
        block_stack.push_back(next_block++);
        }
        block_incr = true;       
        break;
    }
    case TOK_FUNCTION: 
        handle_function(outFile, node);
        break;

    case TOK_PROTOTYPE: 
        handle_prototype(outFile, node);
        break;

    case TOK_STRUCT: 
        handle_struct(outFile, node);
        break;

    case TOK_VARDECL: 
        if (!handle_vardecl(outFile, node)) {
            return false;
        }
        break;

    default: 
        break;
    }

    for(auto child: node->children) {
        bool ret = create_identifiers(outFile, child);
        if (ret == false) {
            return false;
        }
    }
    try {
       bool rc = type_check(node);
        if (!rc) {
            return false;
        }
    }
    catch(...) {
    }
    if (node->symbol == TOK_BLOCK ||
      node->symbol == TOK_FUNCTION ||
      node->symbol == TOK_PROTOTYPE) {
        symbol_stack.pop_back();
        block_stack.pop_back();
        // printf("!!!create_identifiers: after pop, 
        // block_stack.back() %d\n", block_stack.back());
    }
    return true;
}

bool is_type_match(attr_bitset atr1, attr_bitset atr2, 
              string struct1, string struct2) {
    if ((atr1[ATTR_null] && atr2[ATTR_struct]) ||
        (atr1[ATTR_struct] && atr2[ATTR_null]) || 
         (atr1[ATTR_null] && atr2[ATTR_string]) || 
         (atr1[ATTR_string] && atr2[ATTR_null]) || 
         (atr1[ATTR_null] && atr2[ATTR_array])  || 
         (atr1[ATTR_array] && atr2[ATTR_null]))
        return true;

    if ((atr1[ATTR_int] != atr2[ATTR_int]) || 
        (atr1[ATTR_string] != atr2[ATTR_string]) || 
        (atr1[ATTR_array] != atr2[ATTR_array]) || 
        (atr1[ATTR_struct] != atr2[ATTR_struct]) || 
        (atr1[ATTR_void] != atr2[ATTR_void])  || 
        ((atr1[ATTR_struct] != atr2[ATTR_struct]) &&
        (struct1 != struct2)))
        return false;

    return true;
}

bool type_check (astree* node) {
    switch(node->symbol) {
    case TOK_TYPEID: {
        string* name = const_cast<string*>(node->lexinfo);
        if (struct_table.find(name) == struct_table.end()) {
            break;
        }
        symbol* str = struct_table.at(name);
        node->attributes[ATTR_struct] = 1;
        node->struct_name = str->struct_name;
        break;
    }
    case TOK_VOID: {
        if (!(node->parent->symbol == TOK_FUNCTION ||
             node->parent->symbol == TOK_PROTOTYPE)) {
            errprintf("%zd:%zd.%zd: error:",
                      " %s is not inside of function\n", 
                      node->lloc.filenr, node->lloc.linenr, 
                      node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }
        break;
    }
    case TOK_VARDECL: {
        astree* n;
        if (node->children[0]->symbol != TOK_ARRAY)
            n = node->children[0]->children[0];
        else
            n = node->children[0]->children[1];

        if (!is_type_match(n->attributes,
                     node->children[1]->attributes,
                     n->struct_name, node->children[1]->struct_name)) {
            errprintf("%zd:%zd.%zd: error: ",
                      "Incompatible type of '%s'\n", 
                      node->lloc.filenr, node->lloc.linenr, 
                      node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }

        break;
    }
    case TOK_RETURNVOID: {
        node->attributes[ATTR_void] = 1;
        astree* n = node;
        while(n != NULL && n->symbol != TOK_FUNCTION) {
            n = n->parent;
        }
        if (!n->children[0]->children[0]->attributes[ATTR_void]) {
            errprintf("%zd:%zd.%zd: error: ",
                     "Incompatible return type of '%s'\n", 
                      node->lloc.filenr, node->lloc.linenr, 
                      node->lloc.offset, node->lexinfo->c_str() );

            return false;
        }
        break;
    }
    case TOK_RETURN: {
        astree* n = node;
        while(n != NULL && n->symbol != TOK_FUNCTION) {
            n = n->parent;
        }
        if (!is_type_match(n->children[0]->children[0]->attributes,
                     node->children[0]->attributes,
                     n->children[0]->children[0]->struct_name,
                     node->children[0]->struct_name)) {
            errprintf("%zd:%zd.%zd: error:", 
                      " Invalid return type of %s\n", 
                      node->lloc.filenr, node->lloc.linenr, 
                      node->lloc.offset, 
                      node->children[0]->lexinfo->c_str() );
            return false;
        }
        break;
    }
    case '=': {
        if (!is_type_match(node->children[0]->attributes,
                     node->children[1]->attributes,
                     node->children[0]->struct_name,
                     node->children[1]->struct_name) ||
           !node->children[0]->attributes[ATTR_lval]) {
            errprintf("%zd:%zd.%zd: error: Invalid assignment %s\n", 
                      node->lloc.filenr, 
                      node->lloc.linenr, 
                      node->lloc.offset, 
                      node->children[0]->lexinfo->c_str() );
            return false;
        }
        node->attributes[ATTR_string] = 
            node->children[0]->attributes[ATTR_string];
        node->attributes[ATTR_int] = 
            node->children[0]->attributes[ATTR_int];
        node->attributes[ATTR_struct] =
            node->children[0]->attributes[ATTR_struct];
        node->attributes[ATTR_array] = 
            node->children[0]->attributes[ATTR_array];
        node->struct_name = node->children[0]->struct_name;
        node->attributes[ATTR_vreg] = 1;
        break;
    }
    case TOK_POS:
    case TOK_NEG: {
        if (!node->children[0]->attributes[ATTR_int]
           && node->children[0]->attributes[ATTR_array]){
            errprintf("%zd:%zd.%zd: error: ",
                      "Invalid invalid unary operator %s\n", 
                      node->lloc.filenr, node->lloc.linenr, 
                      node->lloc.offset, 
                      node->children[0]->lexinfo->c_str() );
            return false;
        }

        node->attributes[ATTR_int] = 1;
        node->attributes[ATTR_vreg] = 1;
        break;
    }
    case TOK_NEW: {
        node->attributes = node->children[0]->attributes;
        node->struct_name = node->children[0]->struct_name;
        node->attributes[ATTR_vreg] = 1;
        break;
    }
    case TOK_CALL: {
        vector<astree*> params;
        for(uint i = 1; i < node->children.size(); ++i) {
            params.push_back(node->children[i]);
        }

        string* name = const_cast<string*>(
            node->children[0]->lexinfo);

        symbol* sym;
        if (identifiers.find(name) != identifiers.end()) {
            sym = identifiers.at(name);
        } else {
            errprintf("%zd:%zd.%zd: error: ", 
                     "%s is not declared\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }

        if (params.size() != sym->parameters->size()) {
            errprintf("%zd:%zd.%zd: error:", 
                     " %s: Number of parameters is incorrect \n", 
                      node->lloc.filenr, node->lloc.linenr, 
                      node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }
        if (params.size() > 0) {
            for(uint i = 0; i < params.size()-1; ++i) {
                if (!is_type_match(params[i]->attributes,
                             sym->parameters->at(i)->attributes,
                             params[i]->struct_name,
                             sym->parameters->at(i)->struct_name)) {
                     errprintf("%zd:%zd.%zd: error: %s: "
                              , "Mismatch fucntion parameters \n",
                              node->lloc.filenr, node->lloc.linenr, 
                              node->lloc.offset, 
                              node->lexinfo->c_str() );
                    return false;
                }
            }
        }
        node->attributes = sym->attributes;
        node->attributes[ATTR_vreg] = 1;
        node->attributes[ATTR_function] = 1;
        node->struct_name = sym->struct_name;
        break;
    }
    case '-':
    case '*':
    case '/':
    case '%':
    case '+': {
        astree* node1 = node->children[0];
        astree* node2 = node->children[1];
        if (!(node1->attributes[ATTR_int] && 
             node2->attributes[ATTR_int]) && 
            (node1->attributes[ATTR_array] || 
              node2->attributes[ATTR_array])) {
            errprintf("%zd:%zd.%zd: error: ",
                     "%s: Incompatible nary types \n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }
        node->attributes[ATTR_int] = 1;
        node->attributes[ATTR_vreg] = 1;
        break;
    }
    case TOK_EQ:
    case TOK_NE:
    case '<':
    case TOK_LE:
    case '>':
    case TOK_GE: {
        if (!is_type_match(node->children[0]->attributes,
                     node->children[1]->attributes, 
                     node->children[0]->struct_name,
                     node->children[1]->struct_name)) {
            errprintf("%zd:%zd.%zd: error: %s: Invalid comparison\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }

        node->attributes[ATTR_int] = 1;
        node->attributes[ATTR_vreg] = 1;
        break;
    }
    case TOK_NEWSTRING: {
        node->attributes[ATTR_vreg] = 1;
        node->attributes[ATTR_string] = 1;
        break;
    }
    case TOK_IDENT: {
        string* name = const_cast<string*>(node->lexinfo);
        symbol* sym = get_symbol(node);
        if (sym == NULL) {
            if (identifiers.find(name) != identifiers.end()) {
                sym = identifiers[name];
            } else {
                errprintf("%zd:%zd.%zd: error:" , 
                         " '%s' was not declared in this scope\n", 
                         node->lloc.filenr, node->lloc.linenr,
                         node->lloc.offset, name->c_str() );
                return false;
            }
        }
        node->attributes = sym->attributes;
        node->struct_name = sym->struct_name;
        break;
    }

    case TOK_INDEX: {
        if (!node->children[1]->attributes[ATTR_int]) {
            errprintf("%zd:%zd.%zd: error: %s: ",
                     "Invalid array access data type\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }

        if (!(node->children[0]->attributes[ATTR_string]) && 
           !(node->children[0]->attributes[ATTR_array])) {
            errprintf("%zd:%zd.%zd: error: ",
                     "%s: Invalid array access\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }
        if (node->children[0]->attributes[ATTR_string]
           && !node->children[0]->attributes[ATTR_array]) {
            node->attributes[ATTR_int] = 1;
        }
        else {
            node->attributes[ATTR_int] = 
                node->children[0]->attributes[ATTR_int];
            node->attributes[ATTR_string] = 
                node->children[0]->attributes[ATTR_string];
            node->attributes[ATTR_struct] = 
                node->children[0]->attributes[ATTR_struct];
        }

        node->attributes[ATTR_array] = 0;
        node->attributes[ATTR_vaddr] = 1;
        node->attributes[ATTR_lval] = 1;
        node->attributes[ATTR_variable] = 0;
        node->struct_name = node->children[1]->struct_name;
        break;
    }
    case '.': {
        if (!node->children[0]->attributes[ATTR_struct]) {
            errprintf("%zd:%zd.%zd: error: ",
                     "%s: Invalid field selection\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }

        string name =(node->children[0]->struct_name);
        if (field_table.find(name) == field_table.end()) {
            errprintf("%zd:%zd.%zd: error: ", 
                     "%s: was not defined previously\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }
        symbol_table* str = field_table.at(name);
        string* field_name = const_cast<string*>(
            node->children[1]->lexinfo);

        node->attributes = str->at(field_name)->attributes;
        node->struct_name = str->at(field_name)->struct_name;
         node->attributes[ATTR_field] = 0;
        node->attributes[ATTR_vaddr] = 1;
        node->attributes[ATTR_lval] = 1;
        node->attributes[ATTR_variable] = 1;
 
        break;
    }
    case TOK_NULL: {
        node->attributes[ATTR_null] = 1;
        node->attributes[ATTR_const] = 1;
        break;
    }
    case TOK_STRINGCON: {
        node->attributes[ATTR_const] = 1;
        node->attributes[ATTR_string] = 1;
        break;
    }
    case TOK_INTCON: {
        node->attributes[ATTR_const] = 1;
        node->attributes[ATTR_int] = 1;
        break;
    }
    case TOK_CHARCON: {
        node->attributes[ATTR_const] = 1;
        node->attributes[ATTR_int] = 1;
        break;
    }
    case TOK_INT: {
        node->attributes[ATTR_int] = 1;
        break;
    }
    case TOK_STRING: {
        node->attributes[ATTR_string] = 1;
        break;
    }
    case TOK_NEWARRAY: {
        attr_bitset attrs = node->children[0]->attributes;
        if (attrs[ATTR_void] || 
           attrs[ATTR_array] || 
           attrs[ATTR_void] || 
          (attrs[ATTR_struct] && 
          node->children[0]->struct_name == "")) {
            errprintf("%zd:%zd.%zd: error: ",
                     "%s: Invalid array type\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            return false;
        }
        if (attrs[ATTR_int])
            node->attributes[ATTR_int] = 1;
        else if (attrs[ATTR_string])
            node->attributes[ATTR_string] = 1;

        node->attributes[ATTR_vreg] = 1;
        node->attributes[ATTR_array] = 1;
        break;
    }
    default: break;
    }
//    printf("calling type_check:
//    \n----------------------------- end\n");
    return true;
}


void init_symbol(astree* node, symbol* sym) {
    sym->filenr = node->lloc.filenr;
    sym->linenr = node->lloc.linenr;
    sym->offset = node->lloc.offset;
    sym->blocknr = block_stack.back();
    size_t tok= node->symbol;
    if (tok == TOK_INT) {
        sym->attributes[ATTR_int] = 1;
    } else if (tok == TOK_STRING) {
        sym->attributes[ATTR_string] = 1;
    } else if (tok == TOK_VOID) {
        sym->attributes[ATTR_void] = 1;
    } else if (tok == TOK_NULL) {
        sym->attributes[ATTR_null] = 1;
    } else if (tok == TOK_TYPEID) {
        sym->attributes[ATTR_struct] = 1;
        sym->struct_name = string(*(node->lexinfo));
    } else {
        ;
    }
}
symbol* new_symbol(astree* node) {
    symbol* sym = new symbol;
    init_symbol(node, sym);
    sym->attributes[ATTR_lval] = 1;
    sym->attributes[ATTR_variable] = 1;
    return sym;
}

symbol* init_function_attr(astree* node) {
    symbol* sym = new symbol;
    if (node->children[0]->symbol != TOK_ARRAY) {
        init_symbol(node->children[0], sym);
    } else {
        init_symbol(node->children[0]->children[0], sym);
    }
    sym->attributes[ATTR_function] = 1;

    return sym;
}

bool is_valid_variable(string* name) {
    bool found = false;
    for(int i = symbol_stack.size()-1; i >= 0; --i) {
        if (symbol_stack[i] == nullptr) {
            continue;
        }
        if (symbol_stack[i]->find(name) != 
            symbol_stack[i]->end()) {
            if (
                symbol_stack[i]->at(name)->attributes[ATTR_function] 
                == 0){
                found = true;
                break;
            }
        }
    }
    if (identifiers.find(name) != identifiers.end()) found = true;
    return found;
}

void write_symbol_to_file (FILE* outFile, symbol* sym, string* name) {
    attr_bitset attr = sym->attributes;
    string list;
    for(uint i = 0; i < block_stack.size()-1; ++i) {
        fprintf(outFile, "   ");
    }
    string string_list = astree::attribute_string(attr, 
        sym->blocknr, sym->struct_name);
    fprintf(outFile, "%s (%zu.%zu.%zu) %s\n", 
            (*name).c_str(), sym->filenr, sym->linenr, 
            sym->offset, string_list.c_str());
}

bool is_return_type_match (symbol* sym1, symbol* sym2) {
    attr_bitset bit1 = sym1->attributes;
    attr_bitset bit2 = sym2->attributes;
   
    if ((bit1[ATTR_int] && bit2[ATTR_int]) || 
        (bit1[ATTR_void] && bit2[ATTR_void]) || 
        (bit1[ATTR_string] && bit2[ATTR_string]))
        return false;
    return true; 
}

symbol* get_symbol (astree* node) {
    string* name = const_cast<string*>(node->lexinfo);
    for(int i = symbol_stack.size()-1; i >= 0; --i) {
        if (symbol_stack[i] == nullptr) {
            continue;
        }
        if (symbol_stack[i]->find(name) != 
           symbol_stack[i]->end()) {
            if(
            symbol_stack[i]->at(name)->attributes[ATTR_function] == 0){
                return symbol_stack[i]->at(name);
            }
        }
    }
    return NULL;
}

