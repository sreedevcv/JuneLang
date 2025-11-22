#include "ASTPrinter.hpp"

#include "Expr.hpp"
#include "Utils.hpp"
#include "Value.hpp"

#include <ostream>
#include <string>
#include <utility>

void print_type_info(jl::Expr* expr, std::ostream& stream)
{
    if (expr->m_type.get() != nullptr) {
        stream << "[" << expr->m_type->to_str();

        if (expr->m_cast_to) {
            stream << ":" << (*expr->m_cast_to)->to_str();
        }

        stream << "] ";
    }
}

jl::ASTPrinter::ASTPrinter()
{
}

std::stringstream jl::ASTPrinter::print(Expr* epxr)
{
    traverse(epxr);
    return std::move(stream);
}

std::stringstream jl::ASTPrinter::print(const std::vector<std::unique_ptr<jl::Stmt>>& stmts)
{
    for (auto& stmt : stmts) {
        traverse(stmt.get());
    }

    return std::move(stream);
}

void jl::ASTPrinter::traverse(Expr* expr)
{
    depth += increment;
    expr->accept(*this);
    depth -= increment;
}

void jl::ASTPrinter::traverse(Stmt* stmt)
{
    depth += increment;
    stmt->accept(*this);
    depth -= increment;
}

std::ostream& jl::ASTPrinter::spacer()
{
    for (int i = 0; i < depth; i++)
        stream << ' ';

    return stream;
}

std::any jl::ASTPrinter::visit_assign_expr(Assign* expr)
{
    spacer();
    stream << "Assign " << expr->m_token.get_lexeme() << " ";
    print_type_info(expr, stream);
    stream << "  {\n";
    traverse(expr->m_expr.get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_binary_expr(Binary* expr)
{
    spacer();
    stream << "Binary: " << expr->m_oper.get_lexeme() << " ";
    print_type_info(expr, stream);
    stream << " {\n";
    traverse(expr->m_left.get());
    traverse(expr->m_right.get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_grouping_expr(Grouping* expr)
{
    spacer();
    stream << "Grouping: ";
    print_type_info(expr, stream);
    stream << " {\n";
    traverse(expr->m_expr.get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_unary_expr(Unary* expr)
{
    spacer();
    stream << "Unary: " << expr->m_oper.get_lexeme() << " ";
    print_type_info(expr, stream);
    stream << " {\n";
    traverse(expr->m_expr.get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_literal_expr(Literal* expr)
{
    auto& value = expr->m_value;

    spacer();
    stream << "Literal: ";

    print_type_info(expr, stream);

    switch (get_type(value)) {
    case Type::INT:
        stream << std::get<int>(value) << '\n';
        return {};
    case Type::FLOAT:
        stream << std::get<double>(value) << '\n';
        return {};
    case Type::STR:
        stream << std::get<std::string>(value) << '\n';
        return {};
    case Type::BOOL:
        stream << (std::get<bool>(value) ? "true" : "false") << '\n';
        return {};
    case Type::CHAR:
        stream << std::get<char>(value) << '\n';
        return {};
    case Type::JNULL:
        stream << "null" << '\n';
        return {};
    default:
        unimplemented();
        break;
    }
}

std::any jl::ASTPrinter::visit_logical_expr(Logical* expr)
{
    spacer();
    stream << "Logical: " << expr->m_oper.get_lexeme() << " ";
    print_type_info(expr, stream);
    stream << " {\n";
    traverse(expr->m_left.get());
    traverse(expr->m_right.get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_variable_expr(Variable* expr)
{
    spacer();
    stream << "VarRef " << expr->m_name.get_lexeme() << " ";
    print_type_info(expr, stream);
    stream << "  {\n";
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_call_expr(Call* expr) { }
std::any jl::ASTPrinter::visit_get_expr(Get* expr) { }
std::any jl::ASTPrinter::visit_set_expr(Set* expr) { }
std::any jl::ASTPrinter::visit_this_expr(This* expr) { }
std::any jl::ASTPrinter::visit_super_expr(Super* expr) { }
std::any jl::ASTPrinter::visit_jlist_expr(JList* expr) { }
std::any jl::ASTPrinter::visit_index_get_expr(IndexGet* expr) { }
std::any jl::ASTPrinter::visit_index_set_expr(IndexSet* expr) { }
std::any jl::ASTPrinter::visit_type_cast_expr(TypeCast* expr) { }

// -----------------------------------STMT---------------------------------

std::any jl::ASTPrinter::visit_expr_stmt(ExprStmt* stmt)
{
    spacer();
    stream << "Expr:  {\n";
    traverse(stmt->m_expr.get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_var_stmt(VarStmt* stmt)
{
    std::string type_info;
    if (stmt->m_data_type) {
        const auto& type = *stmt->m_data_type;
        type_info += "[" + type.name;
        if (type.is_array) {
            type_info += "[";
            if (type.size)
                type_info += std::to_string(*type.size);
            type_info += "]";
        }
        type_info += "]";
    }

    spacer();
    stream << "VarStmt: " << stmt->m_name.get_lexeme() << ' ' << type_info << " {\n";
    if (stmt->m_initializer)
        traverse((*stmt->m_initializer).get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_empty_stmt(EmptyStmt* stmt)
{
    spacer();
    stream << "EmptyStmt:  {\n";
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_block_stmt(BlockStmt* stmt)
{
    spacer();
    stream << "Block:  {\n";

    for (auto& s : stmt->m_statements) {
        traverse(s.get());
    }

    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_if_stmt(IfStmt* stmt)
{
    spacer();
    stream << "Block:  {\n";
    spacer();
    stream << "Condition: \n";
    traverse(stmt->m_condition.get());
    spacer();
    stream << "Then Stmt: \n";
    traverse(stmt->m_then_stmt.get());
    if (stmt->m_else_stmt) {
        spacer();
        stream << "Else Stmt: \n";
        traverse(stmt->m_then_stmt.get());
    }
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_print_stmt(PrintStmt* stmt) { }
std::any jl::ASTPrinter::visit_while_stmt(WhileStmt* stmt) { }
std::any jl::ASTPrinter::visit_func_stmt(FuncStmt* stmt) { }
std::any jl::ASTPrinter::visit_return_stmt(ReturnStmt* stmt) { }
std::any jl::ASTPrinter::visit_class_stmt(ClassStmt* stmt) { }
std::any jl::ASTPrinter::visit_for_each_stmt(ForEachStmt* stmt) { }
std::any jl::ASTPrinter::visit_break_stmt(BreakStmt* stmt) { }
std::any jl::ASTPrinter::visit_extern_stmt(ExternStmt* stmt) { }
