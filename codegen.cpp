#include "codegen.hpp"
#include "parser.hpp"
#include <iostream>
#include <string>
#include <vector>

std::string c_type(Type type) {
    std::string base;
    
    // Determine base type name
    switch (type.type_id) {
        case DefaultType_u8:   base = "uint8_t"; break;
        case DefaultType_u16:  base = "uint16_t"; break;
        case DefaultType_u32:  base = "uint32_t"; break;
        case DefaultType_u64:  base = "uint64_t"; break;
        case DefaultType_i8:   base = "int8_t"; break;
        case DefaultType_i16:  base = "int16_t"; break;
        case DefaultType_i32:  base = "int32_t"; break;
        case DefaultType_i64:  base = "int64_t"; break;
        case DefaultType_f32:  base = "float"; break;
        case DefaultType_f64:  base = "double"; break;
        case DefaultType_char8:  base = "char"; break;
        case DefaultType_string: base = "char*"; break;
        default:               base = "void"; break;
    }

    // Apply modifiers (e.g., []string -> char**)
    for (auto mod : type.mods) {
        if (mod.kind == Modifier_Arr || mod.kind == Modifier_Slice) {
            base += "*";
        }
    }

    return base;
}

std::string op_to_str(Operator op) {
    switch (op) {
        case Op_Add:    return "+";
        case Op_Sub:    return "-";
        case Op_Mul:    return "*";
        case Op_Div:    return "/";
        case Op_Mod:    return "%";
        case Op_Assign: return "=";
        default:        return "";
    }
}

void emit_expr(std::ostream &out, Expr *expr) {
    if (!expr) return;

    switch (expr->kind) {
        case Expr_Atom:
            out << expr->at.value;
            break;
        case Expr_Operator:
            if (expr->op != Op_Assign) out << "(";
            emit_expr(out, expr->left);
            out << " " << op_to_str(expr->op) << " ";
            emit_expr(out, expr->right);
            if (expr->op != Op_Assign) out << ")";
            break;
        case Expr_FuncCall:
            out << expr->func_call.name << "(";
            for (size_t i = 0; i < expr->func_call.args.size(); ++i) {
                emit_expr(out, expr->func_call.args[i]);
                if (i < expr->func_call.args.size() - 1) out << ", ";
            }
            out << ")";
            break;
        default:
            break;
    }
}

void emit_c_ir(std::ostream &out, const FunctionDefinition &f) {
    out << "#include <stdlib.h>\n"
        << "#include <stdio.h>\n"
        << "#include <stdint.h>\n"
        << "#include <stdbool.h>\n\n";

    // Fix: main must return int in C
    if (f.name == "main") {
        out << "int";
    } else if (f.return_type.has_value()) {
        out << c_type(f.return_type.value());
    } else {
        out << "void";
    }

    out << " " << f.name << "(";
    for (size_t i = 0; i < f.parameters.size(); ++i) {
        // Fix: first parameter of main (argc) must be int in C
        if (f.name == "main" && i == 0) {
            out << "int " << f.parameters[i].name;
        } else {
            out << c_type(f.parameters[i].type) << " " << f.parameters[i].name;
        }
        if (i < f.parameters.size() - 1) out << ", ";
    }
    out << ") {\n";

    for (auto *stmt : f.statements) {
        if (!stmt) continue;
        out << "    ";
        switch (stmt->kind) {
            case Statement_Declaration:
                out << c_type(stmt->declaration_statement->type) << " " 
                    << stmt->declaration_statement->name << ";";
                break;
            case Statement_Definition:
                out << c_type(stmt->definition_statement->type) << " " 
                    << stmt->definition_statement->name << " = ";
                emit_expr(out, stmt->definition_statement->right);
                out << ";";
                break;
            case Statement_Expression:
                emit_expr(out, stmt->expression_statement->root);
                out << ";";
                break;
            case Statement_Return:
                out << "return ";
                if (stmt->return_statement && stmt->return_statement->root) {
                    emit_expr(out, stmt->return_statement->root);
                }
                out << ";";
                break;
            default:
                break;
        }
        out << "\n";
    }
    out << "}\n";
}
