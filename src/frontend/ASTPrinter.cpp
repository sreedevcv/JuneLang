#include "ASTPrinter.hpp"

#include "Expr.hpp"
#include "TypeInfo.hpp"
#include "Utils.hpp"
#include "Value.hpp"

#include <ostream>
#include <string>
#include <utility>

void print_node_type(jl::Expr* expr, std::ostream& stream)
{
    if (expr->m_type != nullptr) {
        stream << "(" << expr->m_type->to_str();

        if (expr->m_cast_to) {
            stream << ":" << (*expr->m_cast_to)->to_str();
        }

        stream << ") ";
    }
}

void print_type_info(const jl::TypeInfo& type_info, std::ostream& stream)
{
    stream << type_info.name;
    if (type_info.is_array) {
        stream << "(";
        if (type_info.size)
            stream << std::to_string(*type_info.size);
        stream << ")";
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
    print_node_type(expr, stream);
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
    print_node_type(expr, stream);
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
    print_node_type(expr, stream);
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
    print_node_type(expr, stream);
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

    print_node_type(expr, stream);

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
    print_node_type(expr, stream);
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
    stream << "VarRef \"" << expr->m_name.get_lexeme() << "\" ";
    print_node_type(expr, stream);
    stream << "\n";
    return {};
}

std::any jl::ASTPrinter::visit_call_expr(Call* expr)
{
    spacer();
    stream << "Call: ";
    print_node_type(expr, stream);
    stream << " {\n";

    spacer();
    stream << "Name: \n";

    traverse(expr->m_callee.get());

    spacer();
    stream << "Args: \n";

    for (auto& arg : expr->m_arguments) {
        traverse(arg.get());
    }

    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_jlist_expr(JList* expr)
{
    spacer();
    stream << "List: ";
    print_node_type(expr, stream);
    stream << " {\n";

    for (auto& e : expr->m_items) {
        traverse(e.get());
    }

    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_index_get_expr(IndexGet* expr)
{
    spacer();
    stream << "Index Get: ";
    print_node_type(expr, stream);
    stream << " {\n";

    spacer();
    stream << "Expr: \n";
    traverse(expr->m_jlist.get());

    spacer();
    stream << "Get Expr: \n";
    traverse(expr->m_index_expr.get());

    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_index_set_expr(IndexSet* expr)
{
    spacer();
    stream << "Index Set: ";
    print_node_type(expr, stream);
    stream << " {\n";

    spacer();
    stream << "Expr: \n";
    traverse(expr->m_jlist.get());

    spacer();
    stream << "Set Expr: \n";
    traverse(expr->m_index_expr.get());

    spacer();
    stream << "Value Expr: \n";
    traverse(expr->m_value_expr.get());

    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_get_expr(Get* expr) { return {}; }
std::any jl::ASTPrinter::visit_set_expr(Set* expr) { return {}; }
std::any jl::ASTPrinter::visit_this_expr(This* expr) { return {}; }
std::any jl::ASTPrinter::visit_super_expr(Super* expr) { return {}; }
std::any jl::ASTPrinter::visit_type_cast_expr(TypeCast* expr) { return {}; }

// -----------------------------------STMT---------------------------------

std::any jl::ASTPrinter::visit_expr_stmt(ExprStmt* stmt)
{
    spacer();
    stream << "Expr: {\n";
    traverse(stmt->m_expr.get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_var_stmt(VarStmt* stmt)
{
    spacer();
    stream << "VarStmt: " << stmt->m_name.get_lexeme() << ' ';
    if (stmt->m_data_type)
        print_type_info(stmt->m_data_type.value(), stream);
    stream << " {\n";
    if (stmt->m_initializer)
        traverse((*stmt->m_initializer).get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_empty_stmt(EmptyStmt* stmt)
{
    spacer();
    stream << "EmptyStmt: {\n";
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_block_stmt(BlockStmt* stmt)
{
    spacer();
    stream << "Block: {\n";

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
    stream << "If Block: {\n";
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

std::any jl::ASTPrinter::visit_while_stmt(WhileStmt* stmt)
{
    spacer();
    stream << "While Block: {\n";
    spacer();
    stream << "Condition: \n";
    traverse(stmt->m_condition.get());
    spacer();
    stream << "Body: \n";
    traverse(stmt->m_body.get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_func_stmt(FuncStmt* stmt)
{
    spacer();
    stream << "Func: " << stmt->m_name.get_lexeme() << '(';

    for (int i = 0; i < static_cast<int>(stmt->m_data_types.size()) - 1; i++) {
        stream << stmt->m_params[i]->get_lexeme() << ": ";
        print_type_info(stmt->m_data_types[i], stream);
        stream << ", ";
    }
    if (stmt->m_data_types.size() > 0) {
        stream << stmt->m_params.back()->get_lexeme() << ": ";
        print_type_info(stmt->m_data_types.back(), stream);
    }

    stream << ") -> ";

    if (stmt->m_return_type)
        print_type_info(stmt->m_return_type.value(), stream);

    stream << " {\n";

    for (auto& s : stmt->m_body) {
        traverse(s.get());
    }

    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_return_stmt(ReturnStmt* stmt)
{
    spacer();
    stream << "Return: {\n";
    if (stmt->m_expr)
        traverse(stmt->m_expr->get());
    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_print_stmt(PrintStmt* stmt)
{
    spacer();
    stream << "Print: {\n";

    // stream << "Condition: \n";
    traverse(stmt->m_expr.get());

    spacer();
    stream << "}\n";
    return {};
}

std::any jl::ASTPrinter::visit_class_stmt(ClassStmt* stmt) { return {}; }
std::any jl::ASTPrinter::visit_for_each_stmt(ForEachStmt* stmt) { return {}; }
std::any jl::ASTPrinter::visit_break_stmt(BreakStmt* stmt) { return {}; }
std::any jl::ASTPrinter::visit_extern_stmt(ExternStmt* stmt) { return {}; }
