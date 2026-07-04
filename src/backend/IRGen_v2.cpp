#include "IRGen_v2.hpp"

#include "Expr.hpp"
#include "LiteralValue.hpp"
#include "Module.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include "ir/AllocateVar.hpp"
#include "value/Variable.hpp"

#include "backend/ir/Binary.hpp"
#include "backend/ir/Call.hpp"
#include "backend/ir/Jump.hpp"
#include "backend/ir/Move.hpp"
#include "backend/ir/Return.hpp"
#include "backend/ir/TypeCast.hpp"
#include "backend/ir/Unary.hpp"
#include "backend/value/Variable.hpp"
#include "frontend/types/Type.hpp"
#include "ir/AllocateList.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/DebugPrint.hpp"
#include "ir/InitLiteral.hpp"
#include "ir/Read.hpp"
#include "ir/Write.hpp"

#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// std::vector<std::unique_ptr<jl::type::Type>> vec;
// auto void_func = std::make_unique<jl::type::Func>(
//     std::move(std::make_unique<jl::type::Builtin>(jl::type::Builtin::VOID)),
//     std::move(vec));

jl::IRGenv2::IRGenv2(TypeContext& type_context)
    : m_type_context(type_context)
    , m_block(nullptr)
{
    auto void_func = m_type_context.create_function(
        type::Func(m_type_context.create_builtin(type::Builtin(type::Builtin::VOID)),
            {}));

    auto function = m_module.create_function("__root__", void_func);
    auto entry_block = function->new_block("entry");
    function->set_current_block(entry_block);

    push_block();
}

jl::Module jl::IRGenv2::generate(Expr* expr)
{
    emit(expr);
    return std::move(m_module);
}

jl::Module jl::IRGenv2::generate(std::vector<std::unique_ptr<jl::Stmt>>& stmts)
{
    emit(stmts);
    return std::move(m_module);
}

jl::value::Variable jl::IRGenv2::emit(Expr* expr)
{
    auto var = std::any_cast<value::Variable>(expr->accept(*this));

    // Insert typecast ir here
    if (expr->m_cast_to) {
        auto dest = m_block->create_varaible(expr->m_cast_to.value());

        m_module.current_function()->add_ir<ir::TypeCast>(
            expr->m_type,
            expr->m_cast_to.value(),
            dest,
            var,
            expr->m_line);

        var = dest;
    }

    return var;
}

void jl::IRGenv2::emit(Stmt* stmt)
{
    stmt->accept(*this);
}

void jl::IRGenv2::emit(std::vector<std::unique_ptr<Stmt>>& stmts)
{
    for (auto& stmt : stmts) {
        emit(stmt.get());
    }
}

void jl::IRGenv2::push_block()
{
    m_env.push(SemanticBlock(m_block, m_module.current_function()));
    m_block = &m_env.top();
}

void jl::IRGenv2::pop_block()
{
    m_env.pop();
    m_block = &m_env.top();
}

void jl::IRGenv2::allocate_variable(value::Variable address_var, const type::Type* type, uint32_t line)
{

    auto curr_block = m_module.current_function()->current_block();
    m_module.current_function()->set_current_block(m_module.current_function()->entry_block());
    m_module.current_function()->add_ir_to_front(ir::AllocateVar(address_var, type, line));
    m_module.current_function()->set_current_block(curr_block);
}

//---------------------------------------------EXPR----------------------------------------------

std::any jl::IRGenv2::visit_assign_expr(Assign* expr)
{
    const auto rhs = emit(expr->m_expr.get());
    const auto addr_var = m_block->lookup_variable(expr->m_token.get_lexeme()).value();
    auto pointee_type = static_cast<const type::Pointer*>(addr_var.type())->m_pointee;
    m_module.current_function()->add_ir<ir::Write>(
        rhs,
        addr_var,
        std::nullopt,
        pointee_type->size(),
        pointee_type->size(),
        expr->m_line);

    return rhs;
}

std::any jl::IRGenv2::visit_binary_expr(Binary* expr)
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
    const auto dest = m_block->create_varaible(expr->m_type);
    const auto is_float = dynamic_cast<const type::Builtin*>(expr->m_left->m_type)->m_primitive == type::Builtin::FLOAT;
    m_module.current_function()->add_ir<ir::Binary>(dest, operand_a, operand_b, operation, is_float, expr->m_oper.get_line());

    return dest;
}

std::any jl::IRGenv2::visit_grouping_expr(Grouping* expr)
{
    return emit(expr->m_expr.get());
}

std::any jl::IRGenv2::visit_unary_expr(Unary* expr)
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
    const auto dest = m_block->create_varaible(expr->m_type);
    m_module.current_function()->add_ir<ir::Unary>(dest, source, operation, expr->m_oper.get_line());
    return dest;
}

std::any jl::IRGenv2::visit_literal_expr(Literal* expr)
{
    auto& value = expr->m_value;
    auto type = expr->m_type;

    switch (get_type(value)) {
    case Type::INT:
        return add_literal_ir<int>(value, type, expr->m_line);
    case Type::FLOAT:
        return add_literal_ir<double>(value, type, expr->m_line);
    case Type::BOOL:
        return add_literal_ir<bool>(value, type, expr->m_line);
    case Type::CHAR:
        return add_literal_ir<char>(value, type, expr->m_line);
    case Type::STR: {
        auto& str = std::get<std::string>(value);
        const auto ptr_var = m_block->create_varaible(expr->m_type);
        // Should I align the stack allocation to 16 bytes??
        const auto list_var = m_block->create_varaible(expr->m_type);
        auto allocate_ir = ir::AllocateList(ptr_var, list_var, 1, str.size(), expr->m_line);
        allocate_ir.set_data(str.data(), str.size());
        m_module.current_function()->add_ir(std::move(allocate_ir));
        return ptr_var;
    } break;
    case Type::JNULL:
    default:
        unimplemented();
        break;
    }

    unimplemented("IRGenv2");
    return {};
}

std::any jl::IRGenv2::visit_variable_expr(Variable* expr)
{
    const auto var = m_block->lookup_variable(expr->m_name.get_lexeme()).value();
    if (var.type()->m_kind == type::Type::FUNC) {
        return var;
    } else {
        auto pointee_type = static_cast<const type::Pointer*>(var.type())->m_pointee;
        auto loaded_value = m_block->create_varaible(pointee_type);
        m_module.current_function()->add_ir<ir::Read>(
            loaded_value,
            var,
            std::nullopt,
            pointee_type->size(),
            pointee_type->size(),
            expr->m_line);
        return loaded_value;
    }
}

std::any jl::IRGenv2::visit_logical_expr(Logical* expr)
{
    auto operation = ir::Binary::LOG_AND;

    if (expr->m_oper.get_tokentype() == Token::OR) {
        operation = ir::Binary::LOG_OR;
    }

    const auto dest = m_block->create_varaible(expr->m_type);
    const auto operand_a = emit(expr->m_left.get());
    const auto operand_b = emit(expr->m_right.get());
    const auto is_float = dynamic_cast<const type::Builtin*>(expr->m_type)->m_primitive == type::Builtin::FLOAT;
    m_module.current_function()->add_ir<ir::Binary>(dest, operand_a, operand_b, operation, is_float, expr->m_line);

    return dest;
}

std::any jl::IRGenv2::visit_call_expr(Call* expr)
{
    std::vector<value::Variable> args;

    for (auto& e : expr->m_arguments) {
        args.push_back(emit(e.get()));
    }

    const auto callee = emit(expr->m_callee.get());
    const auto dest = m_block->create_varaible(expr->m_type);

    m_module.current_function()->add_ir<ir::Call>(m_func_vars.at(callee), std::move(args), dest, expr->m_paren.get_line());

    return dest;
}

std::any jl::IRGenv2::visit_jlist_expr(JList* expr)
{
    const auto list_type = dynamic_cast<const type::List*>(expr->m_type);
    const auto elem_count = expr->m_items.size() + expr->m_extra_item_count.value_or(0);
    const auto elem_size = list_type->m_elem_type->size();
    const auto total_size = elem_count * elem_size;
    const auto ptr_var = m_block->create_varaible(list_type);
    const auto list_var = m_block->create_varaible(list_type);
    auto allocate_ir = ir::AllocateList(ptr_var, list_var, elem_size, elem_count, expr->m_line);

    // allocate_ir.set_data(expr->m_items.data(), expr->m_items.size() * list_type->m_elem_type->size());
    m_module.current_function()->add_ir(std::move(allocate_ir));

    static const auto int_type = type::Builtin(type::Builtin::INT);
    for (uint32_t i = 0; i < expr->m_items.size(); i++) {
        // Compile the item
        auto& elem = expr->m_items[i];
        const auto elem_var = emit(elem.get());
        // Create a var for storing the offset to the item
        const auto offset = m_block->create_varaible(&int_type);
        // Move the offset value(i) to a literal
        m_module.current_function()->add_ir<ir::InitLiteral>(
            LiteralValue(i),
            offset,
            expr->m_right_brace.get_line());

        m_module.current_function()->add_ir<ir::Write>(
            elem_var,
            ptr_var,
            offset,
            elem_size,
            elem_size,
            expr->m_right_brace.get_line());
    }

    return ptr_var;
}

std::any jl::IRGenv2::visit_index_get_expr(IndexGet* expr)
{
    const auto list_var = emit(expr->m_jlist.get());
    const auto offset_var = emit(expr->m_index_expr.get());
    const auto size = expr->m_type->size();
    const auto dest = m_block->create_varaible(expr->m_type);

    m_module.current_function()->add_ir<ir::Read>(dest, list_var, offset_var, size, size, expr->m_line);

    return dest;
}

std::any jl::IRGenv2::visit_index_set_expr(IndexSet* expr)
{
    const auto list_var = emit(expr->m_jlist.get());
    const auto offset_var = emit(expr->m_index_expr.get());
    const auto src_var = emit(expr->m_value_expr.get());
    const auto size = expr->m_type->size();

    m_module.current_function()->add_ir<ir::Write>(src_var, list_var, offset_var, size, size, expr->m_line);

    return src_var;
}

std::any jl::IRGenv2::visit_type_cast_expr(TypeCast* expr)
{
    unimplemented("IRGenv2");
    return {};
}

std::any jl::IRGenv2::visit_get_expr(Get* expr)
{
    unimplemented("IRGenv2");
    return {};
}
std::any jl::IRGenv2::visit_set_expr(Set* expr)
{
    unimplemented("IRGenv2");
    return {};
}
std::any jl::IRGenv2::visit_this_expr(This* expr)
{
    unimplemented("IRGenv2");
    return {};
}
std::any jl::IRGenv2::visit_super_expr(Super* expr)
{
    unimplemented("IRGenv2");
    return {};
}

std::any jl::IRGenv2::visit_print_stmt(PrintStmt* stmt)
{
    const auto var = emit(stmt->m_expr.get());
    const auto builtin = dynamic_cast<const type::Builtin*>(stmt->m_expr.get()->m_type);

    if (builtin) {
        m_module.current_function()->add_ir<ir::DebugPrint>(var, false, builtin->m_primitive, 0, stmt->m_line);
    } else {
        const auto list = dynamic_cast<const type::List*>(stmt->m_expr.get()->m_type);
        const auto builtin = dynamic_cast<const type::Builtin*>(list->m_elem_type);
        m_module.current_function()->add_ir<ir::DebugPrint>(var, true, builtin->m_primitive, builtin->size(), stmt->m_line);
    }

    return {};
}

// --------------------------------------------STMT----------------------------------------

std::any jl::IRGenv2::visit_var_stmt(VarStmt* stmt)
{
    if (!stmt->m_initializer) {
        // TODO::We can may be get the type data necessary to generate an allocate var
        // by adding a type field to VarStmt from the semantic analysis pass
        unimplemented("Untyped variable initializations");
    }

    const auto src = emit(stmt->m_initializer->get());
    auto pointer = m_type_context.create_pointer(type::Pointer(stmt->m_initializer->get()->m_type));
    const auto addr_var = m_block->create_named_variable(stmt->m_name.get_lexeme(), pointer);
    // m_module.current_function()->add_ir<ir::AllocateVar>(addr_var, src.type(), stmt->m_line);
    allocate_variable(addr_var, src.type(), stmt->m_line);
    m_module.current_function()->add_ir<ir::Write>(
        src,
        addr_var,
        std::nullopt,
        src.type()->size(),
        src.type()->size(),
        stmt->m_line);

    return {};
}

std::any jl::IRGenv2::visit_expr_stmt(ExprStmt* stmt)
{
    emit(stmt->m_expr.get());
    return {};
}

std::any jl::IRGenv2::visit_block_stmt(BlockStmt* stmt)
{
    push_block();
    emit(stmt->m_statements);
    pop_block();
    return {};
}

std::any jl::IRGenv2::visit_empty_stmt(EmptyStmt* stmt) { return {}; }

std::any jl::IRGenv2::visit_if_stmt(IfStmt* stmt)
{
    auto condition = emit(stmt->m_condition.get());
    auto if_block = m_module.current_function()->new_block("cond.true");
    auto else_block = m_module.current_function()->new_block("cond.false");

    m_module.current_function()->add_ir<ir::CondJump>(condition, if_block, else_block, stmt->m_line);

    // Evaluate the if block
    m_module.current_function()->set_current_block(if_block);
    emit(stmt->m_then_stmt.get());

    if (stmt->m_else_stmt) {
        auto after_block = m_module.current_function()->new_block("cond.after");

        if (m_module.current_function()->current_block()->get_terminator() == nullptr) {
            m_module.current_function()->add_ir<ir::Jump>(after_block, stmt->m_line);
        }

        m_module.current_function()->set_current_block(else_block);
        emit(stmt->m_else_stmt->get());

        if (m_module.current_function()->current_block()->get_terminator() == nullptr) {
            m_module.current_function()->add_ir<ir::Jump>(after_block, stmt->m_line);
        }

        m_module.current_function()->set_current_block(after_block);
    } else {
        if (m_module.current_function()->current_block()->get_terminator() == nullptr) {
            m_module.current_function()->add_ir<ir::Jump>(else_block, stmt->m_line);
        }

        m_module.current_function()->set_current_block(else_block);
    }

    return {};
}

std::any jl::IRGenv2::visit_while_stmt(WhileStmt* stmt)
{

    auto condition_block = m_module.current_function()->new_block("while.cond");
    auto while_block = m_module.current_function()->new_block("while.body");
    auto after_block = m_module.current_function()->new_block("while.after");

    m_module.current_function()->add_ir<ir::Jump>(condition_block, stmt->m_line);
    m_module.current_function()->set_current_block(condition_block);
    const auto condition = emit(stmt->m_condition.get());

    m_module.current_function()->add_ir<ir::CondJump>(condition, while_block, after_block, stmt->m_line);
    m_module.current_function()->set_current_block(while_block);
    emit(stmt->m_body.get());

    if (m_module.current_function()->current_block()->get_terminator() == nullptr) {
        m_module.current_function()->add_ir<ir::Jump>(condition_block, stmt->m_line);
    }

    m_module.current_function()->set_current_block(after_block);
    return {};
}

std::any jl::IRGenv2::visit_func_stmt(FuncStmt* stmt)
{
    const auto var = m_block->create_named_variable(stmt->m_name.get_lexeme(), stmt->m_type);
    m_func_vars.insert({ var, stmt->m_name.get_lexeme() });

    // Upcast unique ptr to Func from Type
    auto type = static_cast<const type::Func*>(stmt->m_type);
    // Set up new context
    auto current_function = m_module.current_function();
    auto function = m_module.create_function(stmt->m_name.get_lexeme(), stmt->m_type);
    auto entry_block = function->new_block("entry");
    function->set_current_block(entry_block);

    push_block();

    // Add new params to the block
    for (uint32_t i = 0; i < stmt->m_params.size(); i++) {
        auto param_type = m_type_context.from_type_info(stmt->m_data_types[i]).value();
        auto input_arg = m_block->create_varaible(param_type);
        function->add_input_arg(input_arg);

        // Creater a pointer type
        auto pointer = m_type_context.create_pointer(type::Pointer(param_type));
        auto addr_var = m_block->create_named_variable(stmt->m_params[i]->get_lexeme(), pointer);
        // Allocate it on the stack
        m_module.current_function()->add_ir<ir::AllocateVar>(addr_var, param_type, stmt->m_line);
        // allocate_variable(addr_var, param_type, stmt->m_line);
        // Store the value
        m_module.current_function()->add_ir<ir::Write>(input_arg,
            addr_var,
            std::nullopt,
            input_arg.type()->size(),
            input_arg.type()->size(),
            stmt->m_line);
    }

    // Compile the body
    emit(stmt->m_body);

    if (m_module.current_function()->current_block()->get_terminator() == nullptr) {
        m_module.current_function()->add_ir<ir::Return>(std::nullopt, 0);
    }

    // Restore the context
    pop_block();

    m_module.set_current_function(current_function);

    return {};
}

std::any jl::IRGenv2::visit_return_stmt(ReturnStmt* stmt)
{
    auto ret_val = stmt->m_expr.transform([this](auto& e) { return emit(e.get()); });
    m_module.current_function()->add_ir<ir::Return>(ret_val, stmt->m_line);
    return {};
}
std::any jl::IRGenv2::visit_class_stmt(ClassStmt* stmt)
{
    unimplemented("IRGenv2");
    return {};
}
std::any jl::IRGenv2::visit_for_each_stmt(ForEachStmt* stmt)
{
    unimplemented("IRGenv2");
    return {};
}
std::any jl::IRGenv2::visit_break_stmt(BreakStmt* stmt)
{
    unimplemented("IRGenv2");
    return {};
}
std::any jl::IRGenv2::visit_extern_stmt(ExternStmt* stmt)
{
    unimplemented("IRGenv2");
    return {};
}
