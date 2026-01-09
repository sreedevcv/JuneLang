#include "IRGen.hpp"

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include "Value.hpp"
#include "backend/ir/Binary.hpp"
#include "backend/ir/Call.hpp"
#include "backend/ir/Jump.hpp"
#include "backend/ir/Move.hpp"
#include "backend/ir/Return.hpp"
#include "backend/ir/TypeCast.hpp"
#include "backend/ir/Unary.hpp"
#include "backend/types/Type.hpp"
#include "backend/value/Variable.hpp"
#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

std::vector<std::unique_ptr<jl::type::Type>> vec;
auto void_func = std::make_unique<jl::type::Func>(
    std::move(std::make_unique<jl::type::Builtin>(jl::type::Builtin::VOID)),
    std::move(vec));

jl::IRGen::IRGen()
    : m_block(nullptr)
    , m_func(std::string("__root__"), std::move(void_func))
{
    push_block();
}

jl::FuncBlock jl::IRGen::generate(Expr* expr)
{
    emit(expr);
    return std::move(m_func);
}

jl::FuncBlock jl::IRGen::generate(std::vector<std::unique_ptr<jl::Stmt>>& stmts)
{
    emit(stmts);
    return std::move(m_func);
}

std::shared_ptr<jl::value::Variable> jl::IRGen::emit(Expr* expr)
{
    auto var = std::any_cast<std::shared_ptr<value::Variable>>(expr->accept(*this));

    // Insert typecast ir here
    if (expr->m_cast_to) {
        auto dest = m_block->create_varaible(m_func.get_current_func_name(), expr->m_cast_to.value().get());

        m_func.add_ir<ir::TypeCast>(
            expr->m_type->clone(),
            expr->m_cast_to.value()->clone(),
            dest,
            var,
            m_func.get_last_line());

        var = dest;
    }

    return var;
}

void jl::IRGen::emit(Stmt* stmt)
{
    stmt->accept(*this);
}

void jl::IRGen::emit(std::vector<std::unique_ptr<Stmt>>& stmts)
{
    for (auto& stmt : stmts) {
        emit(stmt.get());
    }
}

void jl::IRGen::push_block()
{
    m_env.push(Block(m_block, m_func.get_var_data()));
    m_block = &m_env.top();
}

void jl::IRGen::pop_block()
{
    m_env.pop();
    m_block = &m_env.top();
}

//---------------------------------------------EXPR----------------------------------------------

std::any jl::IRGen::visit_assign_expr(Assign* expr)
{
    const auto temp_var = emit(expr->m_expr.get());
    const auto stack_var = m_block->lookup_variable(expr->m_token.get_lexeme()).value();

    m_func.add_ir<ir::Move>(temp_var, stack_var, expr->m_token.get_line());
    m_block->set_variable_size(stack_var.get(), temp_var.get());

    return stack_var;
}
std::any jl::IRGen::visit_binary_expr(Binary* expr)
{
    ir::Binary::Operation operation;

    switch (expr->m_oper.get_tokentype()) {
    case Token::PLUS:
        operation = ir::Binary::PLUS;
        break;
    case Token::MINUS:
        operation = ir::Binary::MINUS;
        break;
    case Token::STAR:
        operation = ir::Binary::STAR;
        break;
    case Token::SLASH:
        operation = ir::Binary::SLASH;
        break;
    case Token::PERCENT:
        operation = ir::Binary::PERCENT;
        break;
    case Token::BIT_AND:
        operation = ir::Binary::BIT_AND;
        break;
    case Token::BIT_OR:
        operation = ir::Binary::BIT_OR;
        break;
    case Token::BIT_XOR:
        operation = ir::Binary::BIT_XOR;
        break;
    case Token::GREATER:
        operation = ir::Binary::GREATER;
        break;
    case Token::LESS:
        operation = ir::Binary::LESS;
        break;
    case Token::GREATER_EQUAL:
        operation = ir::Binary::GREATER_EQUAL;
        break;
    case Token::LESS_EQUAL:
        operation = ir::Binary::LESS_EQUAL;
        break;
    case Token::EQUAL_EQUAL:
        operation = ir::Binary::EQUAL_EQUAL;
        break;
    case Token::BANG_EQUAL:
        operation = ir::Binary::BANG_EQUAL;
        break;
    default:
        unimplemented();
    }

    const auto operand_a = emit(expr->m_left.get());
    const auto operand_b = emit(expr->m_right.get());
    const auto dest = m_block->create_varaible(m_func.get_current_func_name(), expr->m_type.get());
    m_func.add_ir<ir::Binary>(dest, operand_a, operand_b, operation, expr->m_oper.get_line());

    return dest;
}

std::any jl::IRGen::visit_grouping_expr(Grouping* expr)
{
    return emit(expr->m_expr.get());
}

std::any jl::IRGen::visit_unary_expr(Unary* expr)
{
    ir::Unary::Operation operation;

    switch (expr->m_oper.get_tokentype()) {
    case Token::MINUS:
        operation = ir::Unary::MINUS;
        break;
    case Token::BANG:
        operation = ir::Unary::BANG;
        break;
    case Token::BIT_NOT:
        operation = ir::Unary::BIT_NOT;
        break;
    default:
        unimplemented();
    }

    const auto source = emit(expr->m_expr.get());
    const auto dest = m_block->create_varaible(m_func.get_current_func_name(), expr->m_type.get());
    m_func.add_ir<ir::Unary>(dest, source, operation, expr->m_oper.get_line());
    return dest;
}

std::any jl::IRGen::visit_literal_expr(Literal* expr)
{
    auto& value = expr->m_value;
    auto type = expr->m_type.get();

    switch (get_type(value)) {
    case Type::INT:
        return add_literal_ir<int>(value, type);
    case Type::FLOAT:
        return add_literal_ir<double>(value, type);
    case Type::BOOL:
        return add_literal_ir<bool>(value, type);
    case Type::CHAR:
        return add_literal_ir<char>(value, type);
    case Type::STR:
    case Type::JNULL:
    default:
        unimplemented();
        break;
    }

    unimplemented("IRGen");
    return {};
}

std::any jl::IRGen::visit_variable_expr(Variable* expr)
{
    const auto var = m_block->lookup_variable(expr->m_name.get_lexeme());
    return var.value();
}

std::any jl::IRGen::visit_logical_expr(Logical* expr)
{
    auto operation = ir::Binary::LOG_AND;

    if (expr->m_oper.get_tokentype() == Token::OR) {
        operation = ir::Binary::LOG_OR;
    }

    const auto dest = m_block->create_varaible(m_func.get_current_func_name(), expr->m_type.get());
    const auto operand_a = emit(expr->m_left.get());
    const auto operand_b = emit(expr->m_right.get());
    m_func.add_ir<ir::Binary>(dest, operand_a, operand_b, operation, expr->m_oper.get_line());

    return dest;
}

std::any jl::IRGen::visit_call_expr(Call* expr)
{
    std::vector<std::shared_ptr<value::Variable>> args;

    for (auto& e : expr->m_arguments) {
        args.push_back(emit(e.get()));
    }

    const auto callee = emit(expr->m_callee.get());
    const auto dest = m_block->create_varaible(m_func.get_current_func_name(), expr->m_type.get());

    m_func.add_ir<ir::Call>(m_func_vars.at(callee), std::move(args), dest, expr->m_paren.get_line());

    return dest;
}

std::any jl::IRGen::visit_get_expr(Get* expr)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_set_expr(Set* expr)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_this_expr(This* expr)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_super_expr(Super* expr)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_jlist_expr(JList* expr)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_index_get_expr(IndexGet* expr)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_index_set_expr(IndexSet* expr)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_type_cast_expr(TypeCast* expr)
{
    unimplemented("IRGen");
    return {};
}

std::any jl::IRGen::visit_print_stmt(PrintStmt* stmt)
{
    unimplemented("IRGen");
    return {};
}

// --------------------------------------------STMT----------------------------------------

std::any jl::IRGen::visit_var_stmt(VarStmt* stmt)
{
    if (stmt->m_initializer) {
        const auto src = emit(stmt->m_initializer->get());
        const auto dest = m_block->create_named_variable(
            m_func.get_current_func_name(),
            stmt->m_name.get_lexeme(),
            stmt->m_initializer->get()->m_type.get());
        m_func.add_ir<ir::Move>(src, dest, stmt->m_name.get_line());
    } else {
        // Note::Right now we will add it as 0, it will be replaced with the actual size
        // during assignments
        m_block->create_named_variable(m_func.get_current_func_name(), stmt->m_name.get_lexeme(), 0);
    }

    return {};
}

std::any jl::IRGen::visit_expr_stmt(ExprStmt* stmt)
{
    emit(stmt->m_expr.get());
    return {};
}

std::any jl::IRGen::visit_block_stmt(BlockStmt* stmt)
{
    push_block();
    emit(stmt->m_statements);
    pop_block();
    return {};
}

std::any jl::IRGen::visit_empty_stmt(EmptyStmt* stmt) { return {}; }

std::any jl::IRGen::visit_if_stmt(IfStmt* stmt)
{
    const auto condition = emit(stmt->m_condition.get());
    const auto then_end_label = m_block->create_label();
    // Jump to end of block if condition is false
    m_func.add_ir<ir::Jump>(then_end_label, condition, m_func.get_last_line());
    // Code to execute if condition is true
    emit(stmt->m_then_stmt.get());

    if (stmt->m_else_stmt) {
        // So that code jumps to the end of the block after the execution of above then condition
        const auto else_end_label = m_block->create_label();
        m_func.add_ir<ir::Jump>(else_end_label, std::nullopt, m_func.get_last_line());
        // Point to come if first if-condition fails
        m_func.add_ir<ir::Label>(then_end_label, m_func.get_last_line());
        // Code to execute if condition is false and else block is present
        emit(stmt->m_else_stmt->get());
        // Point to come if the after execution of then-block
        m_func.add_ir<ir::Label>(else_end_label, m_func.get_last_line());
    } else {
        m_func.add_ir<ir::Label>(then_end_label, m_func.get_last_line());
    }

    return {};
}

std::any jl::IRGen::visit_while_stmt(WhileStmt* stmt)
{
    const auto loop_start_label = m_block->create_label();
    const auto loop_end_label = m_block->create_label();

    // Place the loop start label
    m_func.add_ir<ir::Label>(loop_start_label, m_func.get_last_line());
    // Generate the condition
    const auto condition = emit(stmt->m_condition.get());
    // Jump to end of while block if guard check fails
    m_func.add_ir<ir::Jump>(loop_end_label, condition, m_func.get_last_line());
    // Generate the body
    emit(stmt->m_body.get());
    // Unconditional jump to start of the loop
    m_func.add_ir<ir::Jump>(loop_start_label, std::nullopt, m_func.get_last_line());
    // Place the loop end label
    m_func.add_ir<ir::Label>(loop_end_label, m_func.get_last_line());

    return {};
}

std::any jl::IRGen::visit_func_stmt(FuncStmt* stmt)
{
    const auto var = m_block->create_named_variable(
        m_func.get_current_func_name(),
        stmt->m_name.get_lexeme(),
        stmt->m_type.get());
    m_func_vars.insert({ var, stmt->m_name.get_lexeme() });

    // Upcast unique ptr to Func from Type
    auto type = std::unique_ptr<type::Func>((type::Func*)stmt->m_type->clone().release());
    // Set up new context
    m_func.push_func(stmt->m_name.get_lexeme(), std::move(type));
    push_block();

    // Add new params to the block
    for (uint32_t i = 0; i < stmt->m_params.size(); i++) {
        auto param_type = type::from_type_info(stmt->m_data_types[i]);
        m_block->create_named_variable(m_func.get_current_func_name(), stmt->m_params[i]->get_lexeme(), param_type.value().get());
    }

    // Compile the body
    emit(stmt->m_body);

    // Restore the context
    m_func.pop_func();
    pop_block();

    return {};
}

std::any jl::IRGen::visit_return_stmt(ReturnStmt* stmt)
{
    auto ret_val = stmt->m_expr.transform([this](auto& e) { return emit(e.get()); });
    m_func.add_ir<ir::Return>(ret_val, stmt->m_keyword.get_line());
    return {};
}
std::any jl::IRGen::visit_class_stmt(ClassStmt* stmt)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_for_each_stmt(ForEachStmt* stmt)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_break_stmt(BreakStmt* stmt)
{
    unimplemented("IRGen");
    return {};
}
std::any jl::IRGen::visit_extern_stmt(ExternStmt* stmt)
{
    unimplemented("IRGen");
    return {};
}
