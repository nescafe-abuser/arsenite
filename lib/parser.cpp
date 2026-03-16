#include <cstdio>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "lexer.h"
#include "parser.hpp"

// Operator Precedence and Associativity Tables
std::unordered_map<Operator, std::string> op_string {
    {Op_Add, "+"}, {Op_Sub, "-"}, {Op_Mul, "*"},
    {Op_Div, "/"}, {Op_Mod, "%"}, {Op_Assign, "="},
};

std::unordered_map<Operator, int> precedence_table = {
    {Op_Assign, 0}, {Op_Add, 1}, {Op_Sub, 1},
    {Op_Mul, 2}, {Op_Div, 2}, {Op_Mod, 3},
};

std::unordered_map<Operator, OpAss> opass_table = {
    {Op_Add, OpAss_Left}, {Op_Sub, OpAss_Left}, {Op_Mul, OpAss_Left},
    {Op_Div, OpAss_Left}, {Op_Mod, OpAss_Left}, {Op_Assign, OpAss_Right}
};

std::unordered_map<TokenKind, Operator> op_table = {
    {Tok_Plus, Op_Add}, {Tok_Minus, Op_Sub}, {Tok_Star, Op_Mul},
    {Tok_FSlash, Op_Div}, {Tok_Percentage, Op_Mod}, {Tok_Equal, Op_Assign},
};

// --- Statement Implementations ---
Statement::Statement() : kind(Statement_Empty) {}
Statement::Statement(DeclarationStatement *s) : kind(Statement_Declaration), declaration_statement(s) {}
Statement::Statement(DefinitionStatement *s) : kind(Statement_Definition), definition_statement(s) {}
Statement::Statement(ExpressionStatement *s) : kind(Statement_Expression), expression_statement(s) {}
Statement::Statement(IfStatement *s) : kind(Statement_If), if_else_statement(s) {}
Statement::Statement(ReturnStatement *s) : kind(Statement_Return), return_statement(s) {}

// --- Expression Implementations ---
Expr::Expr(ArrIndex ar) : kind(Expr_ArrIndex), arr_index(ar) {}
Expr::Expr(FuncCall fn) : kind(Expr_FuncCall), func_call(fn) {}
Expr::Expr(Atom at) : kind(Expr_Atom), at(at) {}
Expr::Expr(Expr *left, Expr *right, Operator op) : kind(Expr_Operator), op(op), left(left), right(right) {}

// --- Parsing Logic ---

bool is_op(Token t) {
    return op_table.find(t.kind) != op_table.end();
}

Expr* parse_primary(Lexer& l);

Expr* parse_expression(Lexer& l, int min_prec) {
    Expr* primary_lhs = parse_primary(l);
    while (true) {
        if (!is_op(lexer_current(l))) break;
        Operator op = op_table[lexer_current(l).kind];
        if (precedence_table[op] < min_prec) break;
        lexer_next(l);
        int next_min_prec = (opass_table[op] == OpAss_Left) ? precedence_table[op] + 1 : precedence_table[op];
        Expr* primary_rhs = parse_expression(l, next_min_prec);
        primary_lhs = new Expr(primary_lhs, primary_rhs, op);
    }
    return primary_lhs;
}

bool lexer_expect(Lexer &l, TokenKind kind) {
    if (lexer_current(l).kind != kind) {
        printf("Expected \"%s\", got \"%s\"\n", lexer_tokenkind_string(kind).c_str(), lexer_current(l).literal.c_str());
        return false;
    }
    lexer_next(l);
    return true;
}

bool parse_type_declaration(Lexer& l, Type& t) {
    // Handle array/slice modifiers first (e.g., []string)
    while (lexer_current(l).kind == Tok_LBracket) {
        Modifier mod{};
        lexer_next(l); // consume '['
        if (lexer_current(l).kind == Tok_Number) {
            mod.kind = Modifier_Arr;
            mod.size = std::stoull(lexer_current(l).literal);
            lexer_next(l);
        } else {
            mod.kind = Modifier_Slice;
        }
        if (!lexer_expect(l, Tok_RBracket)) return false;
        t.mods.push_back(mod);
    }

    // Parse the base type
    switch (lexer_current(l).kind) {
        case Tok_u8:     t.type_id = DefaultType_u8;     break;
        case Tok_u16:    t.type_id = DefaultType_u16;    break;
        case Tok_u32:    t.type_id = DefaultType_u32;    break;
        case Tok_u64:    t.type_id = DefaultType_u64;    break;
        case Tok_i8:     t.type_id = DefaultType_i8;     break;
        case Tok_i16:    t.type_id = DefaultType_i16;    break;
        case Tok_i32:    t.type_id = DefaultType_i32;    break;
        case Tok_i64:    t.type_id = DefaultType_i64;    break;
        case Tok_f32:    t.type_id = DefaultType_f32;    break;
        case Tok_f64:    t.type_id = DefaultType_f64;    break;
        case Tok_char8:  t.type_id = DefaultType_char8;  break;
        case Tok_string: t.type_id = DefaultType_string; break; // Correctly maps to string
        default: return false;
    }
    lexer_next(l);
    return true;
}

bool parse_statement(Lexer& l, Statement *s) {
    switch (lexer_current(l).kind) {
        case Tok_let: {
            lexer_next(l);
            std::string name = lexer_current(l).literal;
            if (!lexer_expect(l, Tok_Identifier)) return false;
            if (!lexer_expect(l, Tok_Colon)) return false;
            Type t;
            if (!parse_type_declaration(l, t)) return false;
            if (lexer_current(l).kind == Tok_Equal) {
                lexer_next(l);
                DefinitionStatement *stmt = new DefinitionStatement();
                stmt->name = name;
                stmt->type = t;
                stmt->right = parse_expression(l, 0);
                s->kind = Statement_Definition;
                s->definition_statement = stmt;
            } else {
                DeclarationStatement *stmt = new DeclarationStatement();
                stmt->name = name;
                stmt->type = t;
                s->kind = Statement_Declaration;
                s->declaration_statement = stmt;
            }
            return lexer_expect(l, Tok_Semicolon);
        }
        case Tok_return: {
            lexer_next(l);
            ReturnStatement *return_stmt = new ReturnStatement();
            return_stmt->root = parse_expression(l, 0);
            s->kind = Statement_Return; // FIXED
            s->return_statement = return_stmt;
            return lexer_expect(l, Tok_Semicolon);
        }
        case Tok_Identifier: {
            ExpressionStatement *expr_stmt = new ExpressionStatement();
            expr_stmt->root = parse_expression(l, 0);
            s->kind = Statement_Expression;
            s->expression_statement = expr_stmt;
            return lexer_expect(l, Tok_Semicolon);
        }
        default: return false;
    }
}

bool parse_function_definition(Lexer& l, FunctionDefinition& f) {
    if (!lexer_expect(l, Tok_proc)) return false;
    f.name = lexer_current(l).literal;
    if (!lexer_expect(l, Tok_Identifier)) return false;
    if (!lexer_expect(l, Tok_LParen)) return false;

    while (lexer_current(l).kind == Tok_Identifier) {
        Parameter p;
        p.name = lexer_current(l).literal;
        lexer_next(l);
        if (!lexer_expect(l, Tok_Colon)) return false;
        if (!parse_type_declaration(l, p.type)) return false;
        f.parameters.push_back(p);
        if (lexer_current(l).kind == Tok_Comma) lexer_next(l);
        else break;
    }

    if (!lexer_expect(l, Tok_RParen)) return false;

    if (lexer_current(l).kind == Tok_Minus) {
        lexer_next(l);
        if (!lexer_expect(l, Tok_CompGreaterThan)) return false;
        Type t;
        if (!parse_type_declaration(l, t)) return false;
        f.return_type = t;
    }

    if (!lexer_expect(l, Tok_LBrace)) return false;
    while (lexer_current(l).kind != Tok_RBrace) {
        Statement *s = new Statement();
        if (!parse_statement(l, s)) break;
        f.statements.push_back(s);
    }
    return lexer_expect(l, Tok_RBrace);
}

Expr* parse_primary(Lexer& l) {
    Token t = lexer_current(l);
    switch (t.kind) {
        case Tok_Identifier: {
            std::string name = t.literal;
            lexer_next(l); // Consume the identifier

            // CRITICAL: Check for '(' to handle printf(...)
            if (lexer_current(l).kind == Tok_LParen) {
                lexer_next(l); // Consume '('
                FuncCall fn;
                fn.name = name;
                
                if (lexer_current(l).kind != Tok_RParen) {
                    while (true) {
                        fn.args.push_back(parse_expression(l, 0));
                        if (lexer_current(l).kind == Tok_Comma) {
                            lexer_next(l);
                        } else {
                            break;
                        }
                    }
                }
                lexer_expect(l, Tok_RParen);
                return new Expr(fn); // Return as a function call expression
            }
            
            // If no '(', it's just a variable access
            Atom a {.kind = Atom_Variable, .value = name, .type{}};
            return new Expr(a);
        }
        case Tok_Number: {
            lexer_next(l);
            Atom a {.kind = Atom_Constant, .value = t.literal, .type{}};
            return new Expr(a);
        }
        case Tok_StringLit: { 
            // This allows printf("format string")
            lexer_next(l);
            Atom a {.kind = Atom_Constant, .value = t.literal, .type{}};
            return new Expr(a);
        }
        case Tok_LParen: {
            lexer_next(l);
            Expr* e = parse_expression(l, 0);
            lexer_expect(l, Tok_RParen);
            return e;
        }
        default:
            lexer_next(l);
            return nullptr;
    }
}
