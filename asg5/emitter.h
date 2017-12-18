// Matthew Tan
// mxtan
// cmps104a
// asg5: emitter.h

#include <stdio.h>

#include "symtable.h"
using namespace std;

extern FILE* out;

string emit_binop(astree* node);
string mangle_variable(astree* node);

string mangle_var_def(astree* node);
string mangle_reg(astree* node);
void mangle_function(astree* node);

string emit_function(astree* node);
string emit_block(astree* node);
void emit_struct(astree* node);
void emit_var(astree* node);
bool emit_oil_code(astree* node);
