#include "CodeGenerator.hpp"

#include "OpCode.hpp"
#include "Operand.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include "Value.hpp"

#include <any>
#include <print>
#include <string>

jl::CodeGenerator::CodeGenerator(std::string& file_name)
    : m_file_name(file_name)
{
}

jl::CodeGenerator::~CodeGenerator()
{
}

void jl::CodeGenerator::generate(std::vector<Stmt*> stmts)
{
    for (auto stmt : stmts) {
        compile(stmt);
    }
}

std::any jl::CodeGenerator::compile(Stmt* stmt)
{
    return stmt->accept(*this);
}

std::any jl::CodeGenerator::compile(Expr* expr)
{
    return expr->accept(*this);
}

void jl::CodeGenerator::disassemble()
{
    std::println("{}", m_chunk.disassemble());
}

/* ======================================================================================================================== */
/* -----------------------------------------------------EXPR--------------------------------------------------------------- */
/* ======================================================================================================================== */

std::any jl::CodeGenerator::visit_binary_expr(Binary* expr)
{
    auto type = expr->m_oper->get_tokentype();

    auto left_var = compile(expr->m_left);
    auto right_var = compile(expr->m_right);
    auto l = std::any_cast<Operand>(left_var);
    auto r = std::any_cast<Operand>(right_var);
    TempVar dest_var;

    switch (type) {
    case Token::PLUS: {
        dest_var = m_chunk.write(OpCode::ADD, l, r);
    } break;
    case Token::MINUS: {
        dest_var = m_chunk.write(OpCode::MINUS, l, r);
    } break;
    case Token::STAR: {
        dest_var = m_chunk.write(OpCode::STAR, l, r);
    } break;
    case Token::SLASH: {
        dest_var = m_chunk.write(OpCode::SLASH, l, r);
    } break;
    case Token::GREATER: {
        dest_var = m_chunk.write(OpCode::GREATER, l, r);
    } break;
    case Token::LESS: {
        dest_var = m_chunk.write(OpCode::LESS, l, r);
    } break;
    case Token::GREATER_EQUAL: {
        dest_var = m_chunk.write(OpCode::GREATER_EQUAL, l, r);
    } break;
    case Token::LESS_EQUAL: {
        dest_var = m_chunk.write(OpCode::LESS_EQUAL, l, r);
    } break;
    case Token::PERCENT: {
        dest_var = m_chunk.write(OpCode::MODULUS, l, r);
    } break;
    case Token::EQUAL_EQUAL: {
        dest_var = m_chunk.write(OpCode::EQUAL, l, r);
    } break;
    case Token::BANG_EQUAL: {
        dest_var = m_chunk.write(OpCode::NOT_EQUAL, l, r);
    } break;
    default:
        unimplemented();
        break;
    }

    return Operand { dest_var };
}

std::any jl::CodeGenerator::visit_grouping_expr(Grouping* expr)
{
    return compile(expr->m_expr);
}

std::any jl::CodeGenerator::visit_unary_expr(Unary* expr)
{
    auto val = std::any_cast<Operand>(compile(expr->m_expr));
    auto oper = expr->m_oper->get_tokentype();
    Operand temp;

    switch (oper) {
    case Token::MINUS:
        temp = m_chunk.write(OpCode::MINUS, val, Nil {});
        break;
    case Token::BANG:
        temp = m_chunk.write(OpCode::NOT, val, Nil {});
        break;
    default:
        unimplemented();
        break;
    }

    return temp;
}

std::any jl::CodeGenerator::visit_literal_expr(Literal* expr)
{
    Operand literal;
    switch (get_type(*expr->m_value)) {
    case Type::INT: {
        literal = std::get<int>(expr->m_value->get());
    } break;
    case Type::FLOAT: {
        literal = std::get<double>(expr->m_value->get());
    } break;
    case Type::BOOL: {
        literal = std::get<bool>(expr->m_value->get());
    } break;
    default:
        unimplemented();
        break;
    }

    return literal;
}

std::any jl::CodeGenerator::visit_logical_expr(Logical* expr)
{
    const auto left = compile(expr->m_left);
    const auto right = compile(expr->m_right);
    const auto l = std::any_cast<Operand>(left);
    const auto r = std::any_cast<Operand>(right);
    const auto oper = expr->m_oper.get_tokentype();
    Operand temp;

    if (oper == Token::OR) {
        temp = m_chunk.write(OpCode::OR, l, r);
    } else {
        temp = m_chunk.write(OpCode::AND, l, r);
    }

    return temp;
}

// Retreive the temp_var associated with variable
// It should have already been seen by visit_var_stmt
std::any jl::CodeGenerator::visit_variable_expr(Variable* expr)
{
    const auto& var_name = expr->m_name.get_lexeme();
    const auto temp_var = m_chunk.look_up_variable(var_name);

    if (temp_var) {
        return Operand { *temp_var };
    } else {
        std::println("The variable({}) doesn't exist in this scope", var_name);
        unimplemented();    // Fix this after implementing function calls
        return Operand { Nil {} };
    }
}

std::any jl::CodeGenerator::visit_assign_expr(Assign* expr)
{
    const auto assignee = std::any_cast<Operand>(compile(expr->m_expr));

    const auto& var_name = expr->m_token.get_lexeme();
    const auto dest_var = m_chunk.look_up_variable(var_name);

    if (dest_var) {
        m_chunk.write_with_dest(OpCode::ASSIGN, assignee, Nil {}, *dest_var);
    } else {
        unimplemented();
    }

    // For now return the dest_var
    // Not sure whether the return value will be used
    return Operand { *dest_var };
}

std::any jl::CodeGenerator::visit_call_expr(Call* expr) { }

std::any jl::CodeGenerator::visit_get_expr(Get* expr) { }
std::any jl::CodeGenerator::visit_set_expr(Set* expr) { }
std::any jl::CodeGenerator::visit_this_expr(This* expr) { }
std::any jl::CodeGenerator::visit_super_expr(Super* expr) { }
std::any jl::CodeGenerator::visit_jlist_expr(JList* expr) { }
std::any jl::CodeGenerator::visit_index_get_expr(IndexGet* expr) { }
std::any jl::CodeGenerator::visit_index_set_expr(IndexSet* expr) { }

std::any jl::CodeGenerator::visit_var_stmt(VarStmt* stmt)
{
    Operand operand = Nil {};
    if (stmt->m_initializer != nullptr) {
        operand = std::any_cast<Operand>(compile(stmt->m_initializer));
    }

    const auto var = m_chunk.store_variable(stmt->m_name.get_lexeme());
    if (stmt->m_initializer != nullptr) {
        m_chunk.write_with_dest(OpCode::ASSIGN, operand, Nil {}, var);
    }

    return Operand { var };
}

std::any jl::CodeGenerator::visit_expr_stmt(ExprStmt* stmt) { }

std::any jl::CodeGenerator::visit_print_stmt(PrintStmt* stmt) { }
std::any jl::CodeGenerator::visit_block_stmt(BlockStmt* stmt) { }
std::any jl::CodeGenerator::visit_empty_stmt(EmptyStmt* stmt) { }
std::any jl::CodeGenerator::visit_if_stmt(IfStmt* stmt) { }
std::any jl::CodeGenerator::visit_while_stmt(WhileStmt* stmt) { }
std::any jl::CodeGenerator::visit_func_stmt(FuncStmt* stmt) { }
std::any jl::CodeGenerator::visit_return_stmt(ReturnStmt* stmt) { }
std::any jl::CodeGenerator::visit_class_stmt(ClassStmt* stmt) { }
std::any jl::CodeGenerator::visit_for_each_stmt(ForEachStmt* stmt) { }
std::any jl::CodeGenerator::visit_break_stmt(BreakStmt* stmt) { }