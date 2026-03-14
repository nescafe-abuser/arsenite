#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "parser.hpp"
#include <iostream>
#include <string>

std::string c_type(Type type);

std::string op_to_str(Operator op);

void emit_expr(std::ostream &out, Expr *expr);

void emit_c_ir(std::ostream &out, const FunctionDefinition &f);

#endif
