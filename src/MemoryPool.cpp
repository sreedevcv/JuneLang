#include "MemoryPool.hpp"
#include "Callable.hpp"
#include "Environment.hpp"
#include "Expr.hpp"
#include "Token.hpp"
#include <vector>

jl::MemoryPool::MemoryPool()
{
    m_head = &m_dummy_ref;
    m_head->m_next = nullptr;
}

void jl::MemoryPool::mark(Expr* expr)
{
    if (expr->m_marked) {
        return;
    }

    expr->m_marked = true;
    expr->accept(*this);
}

void jl::MemoryPool::mark(Stmt* stmt)
{
    if (stmt == nullptr || stmt->m_marked) {
        return;
    }

    stmt->m_marked = true;
    stmt->accept(*this);
}

void jl::MemoryPool::mark(JlValue* value)
{
    if (value->m_marked) {
        return;
    }

    value->m_marked = true;

    if (is_jlist(value)) {
        for (auto e : *std::get<std::vector<Expr*>*>(value->get())) {
            mark(e);
        }
    } else if (is_callable(value)) {
        auto callable = std::get<Callable*>(value->get());
        mark(callable);
    } else if (is_instance(value)) {
        auto instance = std::get<Instance*>(value->get());
        mark(instance->m_class);

        for (auto& [key, value] : instance->m_fields) {
            mark(value);
        }
    }
}

void jl::MemoryPool::mark(Callable* callable)
{
    if (callable->m_marked) {
        return;
    }

    callable->m_marked = true;

    if (dynamic_cast<FunctionCallable*>(callable)) {
        auto fcallable = static_cast<FunctionCallable*>(callable);
        mark(fcallable->m_declaration);
        mark(fcallable->m_closure); // Delete all the variable defined for the callable
    } else if (dynamic_cast<ClassCallable*>(callable)) {
        auto ccallable = static_cast<ClassCallable*>(callable);

        for (auto& [key, value] : ccallable->m_methods) {
            mark(value);
        }

        if (ccallable->m_super_class != nullptr) {
            mark(ccallable->m_super_class);
        }
    }
}

void jl::MemoryPool::mark(Environment* env)
{
    if (env == nullptr || env->m_marked) {
        return;
    }

    env->m_marked = true;
    for (auto& [key, value] : env->m_values) {
        mark(value);
    }

    if (env->m_enclosing != nullptr) {
        mark(env->m_enclosing);
    }
}

std::any jl::MemoryPool::visit_assign_expr(Assign* expr)
{
    mark(expr->m_expr);
    return nullptr;
}

std::any jl::MemoryPool::visit_binary_expr(Binary* expr)
{
    mark(expr->m_left);
    mark(expr->m_right);
    return nullptr;
}

std::any jl::MemoryPool::visit_grouping_expr(Grouping* expr)
{
    mark(expr->m_expr);
    return nullptr;
}

std::any jl::MemoryPool::visit_unary_expr(Unary* expr)
{
    mark(expr->m_expr);
    return nullptr;
}

std::any jl::MemoryPool::visit_literal_expr(Literal* expr)
{
    mark(expr->m_value);
    return nullptr;
}

std::any jl::MemoryPool::visit_variable_expr(Variable* expr)
{
    return nullptr;
}

std::any jl::MemoryPool::visit_logical_expr(Logical* expr)
{
    mark(expr->m_left);
    mark(expr->m_right);
    return nullptr;
}

std::any jl::MemoryPool::visit_call_expr(Call* expr)
{
    mark(expr->m_callee);
    for (auto e : expr->m_arguments) {
        mark(e);
    }
    return nullptr;
}

std::any jl::MemoryPool::visit_get_expr(Get* expr)
{
    mark(expr->m_object);
    return nullptr;
}

std::any jl::MemoryPool::visit_set_expr(Set* expr)
{
    mark(expr->m_object);
    mark(expr->m_value);
    return nullptr;
}

std::any jl::MemoryPool::visit_this_expr(This* expr)
{
    return nullptr;
}

std::any jl::MemoryPool::visit_super_expr(Super* expr)
{
    return nullptr;
}

std::any jl::MemoryPool::visit_jlist_expr(JList* expr)
{
    for (auto e : expr->m_items) {
        mark(e);
    }
    return nullptr;
}

std::any jl::MemoryPool::visit_index_get_expr(IndexGet* expr)
{
    mark(expr->m_index_expr);
    mark(expr->m_jlist);
    return nullptr;
}

std::any jl::MemoryPool::visit_index_set_expr(IndexSet* expr)
{
    mark(expr->m_index_expr);
    mark(expr->m_jlist);
    mark(expr->m_value_expr);
    return nullptr;
}

// Stmt Visitors

std::any jl::MemoryPool::visit_print_stmt(PrintStmt* stmt)
{
    mark(stmt->m_expr);
    return nullptr;
}

std::any jl::MemoryPool::visit_expr_stmt(ExprStmt* stmt)
{
    mark(stmt->m_expr);
    return nullptr;
}

std::any jl::MemoryPool::visit_var_stmt(VarStmt* stmt)
{
    mark(stmt->m_initializer);
    return nullptr;
}

std::any jl::MemoryPool::visit_block_stmt(BlockStmt* stmt)
{
    for (auto s : stmt->m_statements) {
        mark(s);
    }
    return nullptr;
}

std::any jl::MemoryPool::visit_empty_stmt(EmptyStmt* stmt)
{
    return nullptr;
}

std::any jl::MemoryPool::visit_if_stmt(IfStmt* stmt)
{
    mark(stmt->m_condition);
    mark(stmt->m_else_stmt);
    mark(stmt->m_then_stmt);
    return nullptr;
}

std::any jl::MemoryPool::visit_while_stmt(WhileStmt* stmt)
{
    mark(stmt->m_body);
    mark(stmt->m_condition);
    return nullptr;
}

std::any jl::MemoryPool::visit_func_stmt(FuncStmt* stmt)
{
    for (auto s : stmt->m_body) {
        mark(s);
    }
    return nullptr;
}

std::any jl::MemoryPool::visit_return_stmt(ReturnStmt* stmt)
{
    mark(stmt->m_expr);
    return nullptr;
}

std::any jl::MemoryPool::visit_class_stmt(ClassStmt* stmt)
{
    mark(stmt->m_super_class);
    for (auto s : stmt->m_methods) {
        mark(s);
    }
    return nullptr;
}

std::any jl::MemoryPool::visit_for_each_stmt(ForEachStmt* stmt)
{
    mark(stmt->m_var_declaration);
    mark(stmt->m_list_expr);
    mark(stmt->m_body);
    return nullptr;
}

std::any jl::MemoryPool::visit_break_stmt(BreakStmt* stmt)
{
    return nullptr;
}
