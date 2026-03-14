#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "parser.hpp"
#include <iostream>
#include <string>

// Maps internal language types to standard C types
std::string c_type(Type type);

// Converts internal operators to C operator strings
std::string op_to_str(Operator op);

// Recursively emits a C expression from an Expr AST node
void emit_expr(std::ostream &out, Expr *expr);

// Processes a FunctionDefinition AST node and outputs C IR to the stream
void emit_c_ir(std::ostream &out, const FunctionDefinition &f);

#endif
