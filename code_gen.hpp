#ifndef _CODE_GEN_H
#define _CODE_GEN_H

#include "AST.hpp"
#include "symbolTable.hpp"
#include "routines.hpp"
#include "emitcode.h"
#include "parser.tab.h"
#include <string>

// Control-flow functions
void generate_code(Node *AST, Node *rAST);
void gc_traverse_sibs(Node *);
void gc_traverse_children(Node *);
void generate_code_by_type(Node *);
void generate_IO(Node *rAST);

// Generate code by type
void generate_function(Node *);
void generate_function_compound(Node *);
void generate_compound(Node *node);
void generate_for_compound(Node *, Node *parent);
void generate_while_compound(Node *, Node *parent);
void generate_embedded_while(Node *node);
void generate_op(Node *);
void generate_assignment(Node *);
void generate_and_or(Node *);
void generate_not(Node *);
void generate_if(Node *);
void generate_while(Node *);
void generate_for(Node *);
void generate_embedded_for(Node *);
void generate_constant(Node *);
void generate_id(Node *);
void generate_call(Node *);
void generate_passed_parms(Node *, int);
void generate_return(Node *);
void generate_Ques(Node *);
void generate_ChSign(Node *);
void generate_break(Node *, Node *);
void generate_sizeof(Node *);
void generate_range(Node *);
void generate_string(Node *);
// void generate_XXX(Node *);

// Miscellaneous
bool load_in(Node *); // For Constants and IdNTs
bool load_arr_assign(Node *array, Node *rhs);
bool load_arr_op(Node *array);
void find_embedded_stmts(Node *, Node *parent);
void find_embedded_stmts_wrapper(Node *, Node *parent);
bool store_var(Node *); // For LHS IdNTs
void generate_init_section();
// void insert_decl(Node *);

// Special
void fix_memory_loops(Node *AST);
void traverse_fix(Node *);

#endif