// Matthew Tan
// mxtan
// cs104a
// asg5: emitter.cpp

#include "emitter.h"
#include "lyutils.h"

using namespace std;
#define Ident "        "

extern FILE* oilFile;
extern vector<astree*> string_stack;

vector<unordered_map<const string*, string>> variable_stacks;
int b_reg_idx = 1;
int i_reg_idx = 1;
int p_reg_idx = 1;
int a_reg_idx = 1;
bool is_main = false;

string mangle_statement(astree* node, bool g_variable) {
    astree* l_child = node->children[0];
    astree* l_l_child = node->children[0]->children[0];
    astree* l_r_child = node->children[0]->children[1];

    string mangled, tmp;
    string arr;

    switch(l_child->symbol) {
    case TOK_INDEX: {
        mangled += emit_block(l_l_child) + "[" 
            + emit_block(l_r_child) + "] = ";
        break;
    }
    case TOK_INT: {
        // printf("mangle_statement: node->blocknr: %d\n", 
        // node->blocknr);
        if(node->blocknr == 0)
            mangled += mangle_variable(l_l_child) + " = ";
        else
            mangled += "int " +
                mangle_variable(l_l_child) + " = ";
        break;
    }
    case TOK_STRING: {
        if(node->blocknr == 0)
            mangled += mangle_variable(l_l_child) + " = ";
        else
            mangled += "char* " +
                mangle_variable(l_l_child) + " = ";
        break;
    }
    case TOK_TYPEID: {
        if(node->blocknr == 0)
            mangled += mangle_variable(l_l_child) + " = ";
        else
            mangled += "struct s_" + *(l_child->struct_name) + "* " +
                mangle_variable(l_l_child) + " = ";
        break;
    }
    case TOK_ARRAY: {
        tmp = "p" + to_string(p_reg_idx++);
        if(node->blocknr != 0) {
            if(l_r_child->attributes[ATTR_int]) {
                arr+= "int";
            } else if(l_r_child->attributes[ATTR_string]
                      && l_r_child->attributes[ATTR_array]) {
                arr+= "char*";
            } else if(l_r_child->attributes[ATTR_string]) {
                arr+= "char";
            } else if(l_r_child->attributes[ATTR_struct]) {
                arr+= "struct " + *(l_l_child->lexinfo) + "*";
            }
            arr += "* " + tmp;
        }
        else {
            arr += mangle_variable(l_r_child);
        }
        mangled += arr + " = ";
        break;
    }
    default: {
            errprintf("%zd:%zd.%zd: error: ",
            "'%s' Invalid symbol\n", 
                     l_child->lloc.filenr, l_child->lloc.linenr, 
                     l_child->lloc.offset, l_child->lexinfo->c_str() );
        break;
    }
    }

    if(node->children.size() > 1) {
        astree* r_child = node->children[1];
        switch(r_child->symbol) {
        case TOK_INDEX: {
            astree* r_l_child = node->children[1]->children[0];
            astree* r_r_child = node->children[1]->children[1];
            mangled += emit_block(r_l_child) +
                "[" + emit_block(r_r_child)
                +"]";
            break;
        }
        case TOK_IDENT: {
            mangled += mangle_variable(r_child);
            break;
        }
        case TOK_NULL: {
            mangled += " "+*(node->lexinfo)+" 0";
        }
        case TOK_INT:
        case TOK_INT_CONST:
        case TOK_STRING:
        case TOK_STR_CONST: {
            mangled += *(r_child->lexinfo);
            break;
        }
        case TOK_LEQ:
        case TOK_EQ:
        case TOK_NEQ:
        case TOK_GEQ:
        case '<':
        case '>':
        case '+':
        case '-':
        case '*':
        case '%':
        case '/': {
            mangled += emit_binop(node->children[1]);
            break;
        }
        case TOK_NEWARRAY: {
            mangled += "xcalloc (" + *(r_child->children[1]->lexinfo) +
                ", sizeof(" + arr + "))";
            break;
        }
        case TOK_CALL: {
            if(g_variable)
                mangled += emit_block(node->children[1]);
            else
                emit_block(node->children[1]);
            break;
        }
        case TOK_NEW: {
            mangled += "xcalloc (1, sizeof (struct s_" +
               *(r_child->struct_name) + "))";
            break;
         }
         case '.': {
            mangled += emit_block(r_child);
            break;
         }
         default: {
            errprintf("%zd:%zd.%zd: error: ",
            "'%s' Invalid symbol\n", 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset, node->lexinfo->c_str() );
            break;
         }
      }
   }
   fprintf(oilFile, "%s%s;\n", Ident, mangled.c_str());
   return tmp;

}

string emit_block(astree* node) {
    // printf("emit_block: node: %p\n", node);
   switch(node->symbol) {
      case TOK_TYPEID: {
         return *(node->struct_name);
      }
      case TOK_VARDECL: {
         string decl = mangle_statement(node, false);
         return decl;
         break;
      }
      case TOK_BLOCK: {
          for(auto i: node->children) {
            emit_block(i);
         }
         break;
      }
      case TOK_WHILE: {
         fprintf(oilFile, "while_%zd_%zd_%zd:;\n",
            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
         string operand;
         if(node->children[0]->attributes[ATTR_lval]) {
            operand = mangle_variable(node->children[0]);
         } else {
            operand = "b" + to_string(b_reg_idx++);
            fprintf(oilFile, "%schar %s = %s;\n", 
                    Ident, operand.c_str(), 
                    emit_block(node->children[0]).c_str());
            emit_block(node->children[0]);
         }
         fprintf(oilFile, "%s", Ident);
         fprintf(oilFile, "if(!%s) goto break_%zd_%zd_%zd;\n", 
                 operand.c_str(), node->lloc.filenr, 
                 node->lloc.linenr, node->lloc.offset);

         emit_block(node->children[1]);

         fprintf(oilFile, "%s", Ident);
         fprintf(oilFile, "goto while_%zd_%zd_%zd;\n",
                 node->lloc.filenr, 
                 node->lloc.linenr, node->lloc.offset);

         fprintf(oilFile, "break_%zd_%zd_%zd:;\n",
                 node->lloc.filenr, 
                 node->lloc.linenr, node->lloc.offset);
         break;
      }
   case TOK_IFELSE: {
       string operand;
       if(node->children[0]->attributes[ATTR_lval]) {
           operand = emit_block(node->children[0]);;
       } else {
           operand = "i" + to_string(i_reg_idx++);
           fprintf(oilFile, "%sint %s = %s;\n", 
                   Ident, operand.c_str(), 
                   emit_block(node->children[0]).c_str());
       }
       fprintf(oilFile, "%sif(!%s) goto else_%zd_%zd_%zd;\n", Ident, 
               operand.c_str(), node->lloc.filenr,
               node->lloc.linenr, node->lloc.offset);
       fprintf(oilFile, "%s", Ident);
       emit_block(node->children[1]);
       fprintf(oilFile, "goto fi_%zd_%zd_%zd;\n",
               node->lloc.filenr, 
               node->lloc.linenr, node->lloc.offset);
       fprintf(oilFile, "else_%zd_%zd_%zd:;\n",
               node->lloc.filenr, 
               node->lloc.linenr, node->lloc.offset);
       fprintf(oilFile, "%s", Ident);
       emit_block(node->children[2]);
       fprintf(oilFile, "fi_%zd_%zd_%zd:;\n",
               node->lloc.filenr, 
               node->lloc.linenr, node->lloc.offset);
       break;
   }
   case TOK_IF: {
       string operand;
       if(node->children[0]->attributes[ATTR_lval]) {
           operand = emit_block(node->children[0]);;
       } else {
           operand = "i" + to_string(i_reg_idx++);
           fprintf(oilFile, "%sint %s = %s;\n", 
                   Ident, operand.c_str(), 
                   emit_block(node->children[0]).c_str());
       }
       fprintf(oilFile, "%sif(!%s) goto fi_%zd_%zd_%zd;\n", Ident, 
               operand.c_str(), node->lloc.filenr,
               node->lloc.linenr, node->lloc.offset);
       emit_block(node->children[1]);
       fprintf(oilFile, "fi_%zd_%zd_%zd:;\n",
               node->lloc.filenr, 
               node->lloc.linenr, node->lloc.offset);
       break;
   }
   case TOK_RET: {
       if (node->children[0]->attributes[ATTR_const]) {
           fprintf(oilFile, "%sreturn %s;\n", Ident, 
                   (node->children[0]->lexinfo)->c_str());
       } else if(node->children[0]->attributes[ATTR_variable]) {
           fprintf(oilFile, "%sreturn %s;\n", Ident, 
                   mangle_variable(node->children[0]).c_str());
       } else {
           fprintf(oilFile, "%sreturn %s;\n", Ident, 
                   emit_block(node->children[0]).c_str());
       }
       break;
   }
   case TOK_RETURNVOID: {
       fprintf(oilFile, "%sreturn;\n", Ident);
       break;
   }

   case TOK_CALL: {
       return emit_function(node);
       break;
   }

   case TOK_NEG:
   case TOK_POS:
   case TOK_EXC: {
       return *(node->lexinfo) 
           + emit_block(node->children[0]);
   }
   case TOK_LEQ:
   case TOK_EQ:
   case TOK_NEQ:
   case TOK_GEQ:
   case '<':
   case '>': {
       return emit_binop(node);
   }
   case '=': {
       astree* node1 = node->children[0];
       astree* node2 = node->children[1];
       fprintf(oilFile, "%s%s = %s;\n", Ident, 
               emit_block(node1).c_str(), 
               emit_block(node2).c_str());
       break;
   }
   case '.': {
       string var_register; 
       string data_type;
       if(node->attributes[ATTR_int]) {
           data_type = "int ";
       } else if(node->attributes[ATTR_string]
                 && node->attributes[ATTR_array]) {
           data_type = "char *";
       } else if(node->attributes[ATTR_string]) {
           data_type = "char ";
       } else if(node->attributes[ATTR_struct]) {
           data_type = "struct s_" 
               + *(node->struct_name) + " *";
       }

       var_register = "a" + to_string(a_reg_idx++);

       fprintf(oilFile, "%s%s*%s = &%s->f_%s_%s;\n", Ident, 
               data_type.c_str(), var_register.c_str(),
               emit_block(node->children[0]).c_str(),
               (node->children[0]->struct_name)->c_str(),
               (node->children[1]->lexinfo)->c_str());
         
       return "(*" + var_register + ")";
   }
   case '+':
   case '-':
   case '*':
   case '%':
   case '/': {
       string l_child = mangle_variable(node->children[0]);
       string r_child = mangle_variable(node->children[1]);

       l_child = emit_block(node->children[0]);
       r_child = emit_block(node->children[1]);
       string var_register = mangle_reg(node);

       fprintf(oilFile, "%sint %s = ", 
              Ident, var_register.c_str());
       fprintf(oilFile, "%s %s %s;\n", l_child.c_str(), 
              (node->lexinfo)->c_str(), r_child.c_str());
       return var_register;
   }
   case TOK_CHAR_CONST:
   case TOK_INT_CONST:
   case TOK_STR_CONST: {
       return *(node->lexinfo);
   }
   case TOK_NULL:{
       return "0";
   }
   case TOK_IDENT: {
       return mangle_variable(node);
   }
   case TOK_NEWARRAY: {
       astree* l_child = node->children[0];
       string arr;
   
       if(l_child->attributes[ATTR_int]) {
           arr+= "int";
       } else if(l_child->attributes[ATTR_string] 
                 && l_child->attributes[ATTR_array]) {
           arr+= "char*";
       } else if(l_child->attributes[ATTR_string]) {
           arr+= "char";
       } else if(l_child->attributes[ATTR_struct]) {
           arr+= "struct " + *(l_child->lexinfo) + "*";
       }

       return "xcalloc ("+emit_block(node->children[1]) +
           ", sizeof(" + arr + "))";
   }
   case TOK_INDEX: {
       string var_register;
       string data_type;
       if(node->attributes[ATTR_int]) {
           data_type = "int *";
       } else if(node->attributes[ATTR_string] 
                 && node->attributes[ATTR_array]) {
           data_type = "char **";
       } else if(node->attributes[ATTR_string]) {
           data_type = "char *";
       } else if(node->attributes[ATTR_struct]) {
           data_type = "struct s_" 
               + *(node->children[0]->struct_name) + " **";
       }

       var_register = "a" + to_string(a_reg_idx++);
       fprintf(oilFile, "%s%s%s = &%s[%s];\n", 
              Ident, data_type.c_str(), 
              var_register.c_str(), 
              emit_block(node->children[0]).c_str(),
              emit_block(node->children[1]).c_str());
       return "(*" + var_register + ")";;
   }
   default: break;
   }
   return "";
}

string mangle_variable(astree* node) {
    // printf("mangle_variable: %s, 
    // node->blocknr: %d\n", 
    // node->lexinfo->c_str(), node->blocknr);
    for(int i = variable_stacks.size()-1; i >= 0; --i) {
        if(variable_stacks[i].find(node->lexinfo) 
            == variable_stacks[i].end()) {
            continue;
        }
        else {
            if (i == (int)node->blocknr)
            {
                // printf("found %s in variable_stacks i: 
                // %d, node->blocknr: %d \n", 
                //   node->lexinfo->c_str(), i, node->blocknr);
                return variable_stacks[i].at(node->lexinfo);
            }
        }
    }
    while(variable_stacks.size() <= uint(node->blocknr)) {
        unordered_map<const string*, string> map;
        variable_stacks.push_back(map);
    }

    string new_var = "_";
    if (is_main)
        node->blocknr = 0;
    if(node->blocknr != 0)
        new_var += to_string(node->blocknr) 
            + "_" + *(node->lexinfo);
    else if(node->attributes[ATTR_field]) {
        new_var += *(node->lexinfo);
    }
    else
        new_var += "_" + *(node->lexinfo);

    pair <const string*, string> var (node->lexinfo, new_var);
    variable_stacks[node->blocknr].insert(var);
    // printf("mangle_variable: node->blocknr %d, new_var: %s\n", 
    // node->blocknr, new_var.c_str());
    return new_var;
}

string mangle_var_def(astree* node) {

    if (node != nullptr && node->children.size() == 0) {
        return "";
    }

    astree* child = (node->symbol == TOK_ARRAY ? 
        node->children[1] : node->children[0]);
    string mangled;
    attr_bitset attr = child->attributes;

    if(attr[ATTR_void]) {
        mangled += "void ";
    }
    if (attr[ATTR_int] == 1) {
        mangled += "int ";
    } 
    if (attr[ATTR_struct] == 1) {
        if(child->parent->symbol == TOK_ARRAY)
            mangled += "struct s_" 
            + mangle_variable(child) + "* ";
        else
            mangled += "struct s_"+ 
                *(node->struct_name) + "* ";
    } 
    if (attr[ATTR_string] == 1) {
        mangled += "char*";
    }

    if(attr[ATTR_array]) {
        mangled += "* ";
    } else {
        mangled += " ";
    }

    if(attr[ATTR_field]) {
        mangled += "f_" + *(child->struct_name) 
            + mangle_variable(child);
    }

    if(attr[ATTR_param]) {
        mangled += mangle_variable(child);
    }

    return mangled;
}

string mangle_reg(astree* node) {
    string res;
    attr_bitset attr = node->attributes;
    if(attr[ATTR_int]) {
        res += "i" + to_string(i_reg_idx);
    } else if(attr[ATTR_string]) {
        res += "s" + to_string(i_reg_idx);
    } else if(attr[ATTR_struct]) {
        res += "p" + to_string(p_reg_idx);
    }
    i_reg_idx++;
    return res;
}


string emit_binop(astree* node) {
   return emit_block(node->children[0])
          + " " + *(node->lexinfo) + " " 
          + emit_block(node->children[1]);
}

string emit_function(astree* node) {
   vector<string> var_registers;
   if(!node->attributes[ATTR_function])
      return emit_block(node);

   for(size_t i = 1; i < node->children.size(); ++i) {
       var_registers.push_back(
       emit_function(node->children[i]));
    }

   string reg;
   if(node->parent->symbol != TOK_VARDECL) {
      if(node->attributes[ATTR_void]) {
         fprintf(oilFile, "%s__%s(", Ident, 
            (node->children[0]->lexinfo)->c_str());
      } else if(node->attributes[ATTR_int]) {
         reg = mangle_reg(node);
         fprintf(oilFile, "%sint %s = __%s (", 
            Ident, reg.c_str(), 
            (node->children[0]->lexinfo)->c_str());
      } else if(node->attributes[ATTR_string]) {
         reg = mangle_reg(node);
         fprintf(oilFile, "%schar* %s = __%s (", Ident, reg.c_str(), 
            (node->children[0]->lexinfo)->c_str());
      } else if(node->attributes[ATTR_struct]) {
         reg = mangle_reg(node);
         fprintf(oilFile, "%sstruct* s_%s = __%s (", 
            Ident, reg.c_str(), 
            (node->children[0]->lexinfo)->c_str());
      }
   } else {
      reg += "__" + *(node->children[0]->lexinfo) + "(";
      if(var_registers.size() > 0)
         reg += var_registers[0];

      for(size_t i = 2; 
          i < node->children.size(); ++i) {
            reg += ", "+ var_registers[i-1];
      }
      reg += ")";
      return reg;
   }

   if(var_registers.size() > 0)
      fprintf(oilFile, "%s", var_registers[0].c_str());

   for(size_t i = 2; i < node->children.size(); ++i) {
      fprintf(oilFile, ", %s", 
          var_registers[i-1].c_str());
   }
   fprintf(oilFile, ");\n");

   return reg;
}

string format_args(astree* node) {
    string args;
    for(size_t i = 0; i < node->children.size(); ++i) {
        args += Ident 
            + mangle_var_def(node->children[i]);
        if(i < node->children.size()-1) 
            args += ",\n";
    }
    return args;
}

void mangle_function(astree* node) {
    astree* var;
    if(node->children[0]->symbol == TOK_ARRAY) {
        var = node->children[0]->children[1];
    } else {
        var = node->children[0]->children[0];
    }
    string func = mangle_var_def(node->children[0]);
    fprintf(oilFile, "%s__%s (\n", 
        func.c_str(), (var->lexinfo)->c_str());

    string arg_pack = format_args(node->children[1]);
    fprintf(oilFile, "%s)\n{\n", arg_pack.c_str());
    for(size_t i = 0; 
        i < node->children[2]->children.size(); ++i) {
        emit_block(node->children[2]->children[i]);
    }
    fprintf(oilFile, "}\n\n");
}

void emit_struct(astree* node) {
    if(node->struct_name == NULL)
        return;

    fprintf(oilFile, "struct s_%s {\n", 
        node->struct_name->c_str());

    if(node->children.size() > 1) {
        astree* list = node->children[1];
        string var;
        for(auto i: list->children) {
            var = mangle_var_def(i);
            fprintf(oilFile, "%s%s;\n", 
                Ident, var.c_str());
        }
    }
    fprintf(oilFile, "};\n");
}

void emit_string (astree* node) {
    fprintf(oilFile, "char* %s = %s;\n", 
            (node->children[0]->children[0]->lexinfo)->c_str(),
            (node->children[1]->lexinfo)->c_str());
}

void emit_var(astree* node) {
    astree* child_0 = node->children[0];
    astree* child = (child_0->symbol == TOK_ARRAY ? 
                     child_0->children[1] : 
                     child_0->children[0]);

    string mangled = mangle_var_def(child_0);
    string name = mangle_variable(child);
    fprintf(oilFile, "%s %s;\n\n", 
        mangled.c_str(), name.c_str());
}

bool emit_oil_code(astree* node) {
    fprintf(oilFile, "#define __OCLIB_C__\n#include \"oclib.oh\"\n\n");

    for (auto child: node->children) {
        if (child->symbol == TOK_STRUCT) {
            emit_struct(child);
        }
    }

    for (auto str: string_stack) {
        emit_string (str);
    }

    vector<astree*> global_variables;
    for(auto child: node->children) {
        if(child->symbol == TOK_VARDECL) {
            if(child->children[0]->symbol != TOK_STRING) {
                emit_var(child);
                global_variables.push_back(child);
            }
        }
    }

    for(auto child: node->children) {
        if(child->symbol == TOK_FUNCTION) {
            mangle_function(child);
        }
    }

    fprintf(oilFile, "void __ocmain (void)\n{\n");
    is_main = true;
    for(auto i: global_variables) {
        // printf("emit_oil, 
        // mangle global variable_stacks: %s\n", 
        // i->lexinfo->c_str());
        mangle_statement(i, true).c_str();
    }

    string status;
    for(auto child: node->children) {
        if (child->symbol != TOK_FUNCTION &&
            child->symbol != TOK_PROTOTYPE &&
            child->symbol != TOK_VARDECL &&
            child->symbol != TOK_STRUCT) {
            status = emit_block(child);
            if(status == "ERROR") return false;
        }
    }
    fprintf(oilFile, "}\n");
    return true;
}
