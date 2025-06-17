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

    switch (type) {
    case Token::PLUS: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);
        
        auto dest_var = m_chunk.write(OpCode::ADD, l, r);
        return Operand { dest_var };
    } break;
    case Token::MINUS: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);

        auto dest_var = m_chunk.write(OpCode::MINUS, l, r);
        return Operand { dest_var };
    } break;
    case Token::STAR: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);

        auto dest_var = m_chunk.write(OpCode::STAR, l, r);
        return Operand { dest_var };
    } break;
    case Token::SLASH: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);
        
        auto dest_var = m_chunk.write(OpCode::SLASH, l, r);
        return Operand { dest_var };
    } break;
    case Token::GREATER: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);

        auto dest_var = m_chunk.write(OpCode::GREATER, l, r);
        return Operand { dest_var };
    } break;
    case Token::LESS: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);

        auto dest_var = m_chunk.write(OpCode::LESS, l, r);
        return Operand { dest_var };
    } break;
    case Token::GREATER_EQUAL: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);

        auto dest_var = m_chunk.write(OpCode::GREATER_EQUAL, l, r);
        return Operand { dest_var };
    } break;
    case Token::LESS_EQUAL: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);

        auto dest_var = m_chunk.write(OpCode::LESS_EQUAL, l, r);
        return Operand { dest_var };
    } break;
    case Token::PERCENT: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);

        auto dest_var = m_chunk.write(OpCode::MODULUS, l, r);
        return Operand { dest_var };
    } break;
    case Token::EQUAL_EQUAL: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);
        
        auto dest_var = m_chunk.write(OpCode::EQUAL, l, r);
        return Operand { dest_var };
    } break;
    case Token::BANG_EQUAL: {
        // auto n1 = std::get<int>(l);
        // auto n2 = std::get<int>(r);
        
        auto dest_var = m_chunk.write(OpCode::NOT_EQUAL, l, r);
        return Operand { dest_var };
    } break;
    default:
    unimplemented();
    break;
}

return TempVar { 0 };
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
    switch (get_type(*expr->m_value)) {
    case jl::Type::INT: {
        return Operand { std::get<int>(expr->m_value->get()) };
    } break;
    default:
        unimplemented();
        break;
    }
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

std::any jl::CodeGenerator::visit_assign_expr(Assign* expr) { }
std::any jl::CodeGenerator::visit_variable_expr(Variable* expr) { }
std::any jl::CodeGenerator::visit_call_expr(Call* expr) { }
std::any jl::CodeGenerator::visit_get_expr(Get* expr) { }
std::any jl::CodeGenerator::visit_set_expr(Set* expr) { }
std::any jl::CodeGenerator::visit_this_expr(This* expr) { }
std::any jl::CodeGenerator::visit_super_expr(Super* expr) { }
std::any jl::CodeGenerator::visit_jlist_expr(JList* expr) { }
std::any jl::CodeGenerator::visit_index_get_expr(IndexGet* expr) { }
std::any jl::CodeGenerator::visit_index_set_expr(IndexSet* expr) { }

std::any jl::CodeGenerator::visit_print_stmt(PrintStmt* stmt) { }
std::any jl::CodeGenerator::visit_expr_stmt(ExprStmt* stmt) { }
std::any jl::CodeGenerator::visit_var_stmt(VarStmt* stmt) { }
std::any jl::CodeGenerator::visit_block_stmt(BlockStmt* stmt) { }
std::any jl::CodeGenerator::visit_empty_stmt(EmptyStmt* stmt) { }
std::any jl::CodeGenerator::visit_if_stmt(IfStmt* stmt) { }
std::any jl::CodeGenerator::visit_while_stmt(WhileStmt* stmt) { }
std::any jl::CodeGenerator::visit_func_stmt(FuncStmt* stmt) { }
std::any jl::CodeGenerator::visit_return_stmt(ReturnStmt* stmt) { }
std::any jl::CodeGenerator::visit_class_stmt(ClassStmt* stmt) { }
std::any jl::CodeGenerator::visit_for_each_stmt(ForEachStmt* stmt) { }
std::any jl::CodeGenerator::visit_break_stmt(BreakStmt* stmt) { }