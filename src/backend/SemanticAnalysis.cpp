#include "SemanticAnalysis.hpp"

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include "Value.hpp"
#include "backend/types/Type.hpp"

#include <format>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

bool validate_ptr_arithmetic(jl::Binary* expr, const jl::type::Type* left, const jl::type::Type* right, std::string m_file_name);
bool are_pointers(jl::type::Type* left, jl::type::Type* right);
bool is_valid_pointer_operand(jl::type::Type* t);
std::pair<jl::type::Builtin::Primitive, jl::type::Builtin::Primitive> apply_numeric_promotion(jl::Binary* expr, jl::type::Builtin* left, jl::type::Builtin* right);
bool is_float_allowed_op(jl::Token::TokenType t);
bool is_boolean_operator(jl::Token::TokenType t);

constexpr auto VOID_CONSTANT = jl::type::Builtin(jl::type::Builtin::VOID);
constexpr auto INT_CONSTANT = jl::type::Builtin(jl::type::Builtin::INT);

jl::SemanticAnalyzer::SemanticAnalyzer(std::string& file_name)
    : m_file_name(file_name)
{
    m_symbol_table.push_back({});
}

bool jl::SemanticAnalyzer::type_check(Expr* expr)
{
    return std::any_cast<bool>(expr->accept(*this));
}

bool jl::SemanticAnalyzer::type_check(Stmt* stmt)
{
    return std::any_cast<bool>(stmt->accept(*this));
}

bool jl::SemanticAnalyzer::type_check(const std::vector<std::unique_ptr<Stmt>>& stmts)
{
    bool result = true;

    for (const auto& stmt : stmts) {
        result &= type_check(stmt.get());
    }

    return result;
}

bool jl::SemanticAnalyzer::is_defined(const std::string& name)
{
    for (int32_t i = m_symbol_table.size() - 1; i >= 0; i--) {
        const auto& map = m_symbol_table[i];

        if (map.contains(name)) {
            return true;
        }
    }

    return false;
}

std::optional<std::unique_ptr<jl::type::Type>>& jl::SemanticAnalyzer::get_variable_type(const std::string& name)
{
    for (uint32_t i = m_symbol_table.size() - 1; i >= 0; i--) {
        auto& map = m_symbol_table[i];

        if (map.contains(name)) {
            return map.at(name);
        }
    }
}

bool jl::SemanticAnalyzer::define_variable(const std::string& name, std::optional<std::unique_ptr<type::Type>> type)
{
    if (m_symbol_table.back().contains(name)) {
        return false;
    }

    m_symbol_table.back().insert({ name, std::move(type) });

    return true;
}

// -----------------------------------EXPR----------------------------------

std::any jl::SemanticAnalyzer::visit_binary_expr(Binary* expr)
{
    // If  type checking failed, return
    if (!type_check(expr->m_left.get()) || !type_check(expr->m_right.get())) {
        return false;
    }

    const auto left_type = expr->m_left.get()->m_type.get();
    const auto right_type = expr->m_right.get()->m_type.get();

    if (are_pointers(left_type, right_type)) {
        return validate_ptr_arithmetic(expr, left_type, right_type, m_file_name);
    }

    // Check both are numbers
    if (!type::is_number(left_type) || !type::is_number(right_type)) {
        ErrorHandler::error(
            m_file_name,
            expr->m_oper.get_line(),
            std::format("Left: {} and right: {} operands are not numbers", left_type->to_str(), right_type->to_str()).c_str());
        return false;
    }

    const auto left = dynamic_cast<type::Builtin*>(left_type);
    const auto right = dynamic_cast<type::Builtin*>(right_type);

    auto [l_type, r_type] = apply_numeric_promotion(expr, left, right);

    if (is_boolean_operator(expr->m_oper.get_tokentype())) {
        expr->m_type = std::make_unique<type::Builtin>(type::Builtin::BOOL);
    } else {
        // Both l_type and t_type are not the same, so just assign one
        expr->m_type = std::make_unique<type::Builtin>(l_type);
    }

    // If both are floats only certain types of opers are allowed
    if (l_type == type::Builtin::FLOAT && r_type == type::Builtin::FLOAT) {
        if (!is_float_allowed_op(expr->m_oper.get_tokentype())) {
            ErrorHandler::error(
                m_file_name,
                expr->m_oper.get_line(),
                std::format("Operation {} is not allowed for floats", expr->m_oper.get_lexeme()).c_str());
            return false;
        }
    }

    return true;
}

std::any jl::SemanticAnalyzer::visit_grouping_expr(Grouping* expr)
{
    const auto res = type_check(expr->m_expr.get());
    if (res) {
        expr->m_type = expr->m_expr->m_type->clone();
    }
    return res;
}

std::any jl::SemanticAnalyzer::visit_unary_expr(Unary* expr)
{
    if (!type_check(expr->m_expr.get())) {
        return false;
    }

    const auto& type = expr->m_expr->m_type.get();

    switch (expr->m_oper.get_tokentype()) {
    case Token::MINUS: {
        if (type::is_number(type) || type->m_kind == type::Type::PTR) {
            expr->m_type = type->clone();
            return true;
        }
    } break;
    case Token::BANG:
        if (type->m_kind == type::Type::BUILTIN && static_cast<type::Builtin*>(type)->m_primitive == type::Builtin::BOOL) {
            expr->m_type = type->clone();
            return true;
        }
        break;
    case Token::BIT_NOT:
        if (type->m_kind == type::Type::BUILTIN && static_cast<type::Builtin*>(type)->m_primitive == type::Builtin::INT) {
            expr->m_type = type->clone();
            return true;
        }
        break;
    default:
        break;
    }

    ErrorHandler::error(
        m_file_name,
        expr->m_oper.get_line(),
        std::format("Operation '{}' not permitted for type: {}", expr->m_oper.get_lexeme(), expr->m_expr->m_type->to_str()).c_str());
    return false;
}

std::any jl::SemanticAnalyzer::visit_literal_expr(Literal* expr)
{
    auto& value = expr->m_value;

    switch (get_type(value)) {
    case Type::INT:
        expr->m_type = std::make_unique<type::Builtin>(type::Builtin::INT);
        return true;
    case Type::FLOAT:
        expr->m_type = std::make_unique<type::Builtin>(type::Builtin::FLOAT);
        return true;
    case Type::BOOL:
        expr->m_type = std::make_unique<type::Builtin>(type::Builtin::BOOL);
        return true;
    case Type::CHAR:
        expr->m_type = std::make_unique<type::Builtin>(type::Builtin::CHAR);
        return true;
    case Type::STR:
        expr->m_type = std::make_unique<type::List>(
            std::make_unique<type::Builtin>(type::Builtin::CHAR),
            std::get<std::string>(value).size());
        return true;
    case Type::JNULL:
    // TODO: Create a pointer literal of value of zero
    default:
        unimplemented();
        break;
    }

    return false;
}

std::any jl::SemanticAnalyzer::visit_logical_expr(Logical* expr)
{
    if (!type_check(expr->m_left.get()) || !type_check(expr->m_right.get())) {
        return false;
    }

    const auto left = expr->m_left.get()->m_type.get();
    const auto right = expr->m_right.get()->m_type.get();

    if (!is_boolean(left) || !is_boolean(right)) {
        ErrorHandler::error(m_file_name, expr->m_oper.get_line(),
            std::format("Operation: {} is only permitted for booleans, here right: {} and left: {}",
                expr->m_oper.get_lexeme(),
                right->to_str(),
                left->to_str())
                .c_str());
        return false;
    }

    expr->m_type = left->clone();
    return true;
}

std::any jl::SemanticAnalyzer::visit_variable_expr(Variable* expr)
{
    if (!is_defined(expr->m_name.get_lexeme())) {
        ErrorHandler::error(m_file_name, expr->m_name.get_line(),
            std::format("Variable {} is undefined", expr->m_name.get_lexeme()).c_str());
        return false;
    }

    auto& type = get_variable_type(expr->m_name.get_lexeme());

    if (!type) {
        ErrorHandler::error(m_file_name, expr->m_name.get_line(),
            std::format("Variable {} is uninitialized", expr->m_name.get_lexeme()).c_str());
        return false;
    }

    expr->m_type = type.value().get()->clone();

    return true;
}

std::any jl::SemanticAnalyzer::visit_assign_expr(Assign* expr)
{
    if (!type_check(expr->m_expr.get()))
        return false;

    if (!is_defined(expr->m_token.get_lexeme())) {
        ErrorHandler::error(m_file_name, expr->m_token.get_line(),
            std::format("Variable {} is undefined", expr->m_token.get_lexeme()).c_str());
        return false;
    }

    auto& type = get_variable_type(expr->m_token.get_lexeme());
    if (expr->m_expr->m_type.get()->equals(&VOID_CONSTANT) || (type && type->get()->equals(&VOID_CONSTANT))) {
        ErrorHandler::error(m_file_name, expr->m_token.get_line(),
            "Assignments involving void values are not allowed");
        return false;
    }

    if (!type) {
        // Variable is untyped as of now
        type = expr->m_expr->m_type.get()->clone();
        expr->m_type = type.value().get()->clone();

        if (type.value()->m_kind == type::Type::LIST) {
            ErrorHandler::error(m_file_name, expr->m_token.get_line(),
                "Late initializations of lists are not allowed");
            return false;
        }

        return true;
    }

    if (type.value()->equals(expr->m_expr.get()->m_type.get())) {
        expr->m_type = type.value().get()->clone();
        return true;
    }

    // Variable and expr types do not match
    ErrorHandler::error(m_file_name, expr->m_token.get_line(),
        std::format("Variable {} of type {} cannot be assigned value of type {}",
            expr->m_token.get_lexeme(), type.value()->to_str(),
            expr->m_expr->m_type.get()->to_str())
            .c_str());
    return false;
}

std::any jl::SemanticAnalyzer::visit_call_expr(Call* expr)
{
    if (!type_check(expr->m_callee.get())) {
        return false;
    }

    const auto type = expr->m_callee.get()->m_type.get();

    // If parsed variable is not a function
    if (type->m_kind != type::Type::FUNC) {
        ErrorHandler::error(m_file_name, expr->m_paren.get_line(),
            std::format("Only functions can be called, found type: {}", type->to_str()).c_str());
        return false;
    }

    const auto func = static_cast<const type::Func*>(type);

    // No. of arguments differ
    if (func->m_param_types.size() != expr->m_arguments.size()) {
        ErrorHandler::error(m_file_name, expr->m_paren.get_line(),
            std::format("Expected {} arguments but found type: {}", func->m_param_types.size(), expr->m_arguments.size()).c_str());
        return false;
    }

    for (int i = 0; i < func->m_param_types.size(); i++) {
        auto& arg = expr->m_arguments[i];

        if (!type_check(arg.get())) {
            return false;
        }

        // Types of declared param and argument differ
        if (!func->m_param_types[i]->equals(arg->m_type.get())) {
            ErrorHandler::error(m_file_name, expr->m_paren.get_line(),
                std::format("Expected argument {} to be of type {}  but found type: {}",
                    i + 1, func->m_param_types[i]->to_str(), arg->m_type->to_str())
                    .c_str());
            return false;
        }
    }

    expr->m_type = func->m_return_type->clone();

    return true;
}

std::any jl::SemanticAnalyzer::visit_jlist_expr(JList* expr)
{
    if (expr->m_items.empty()) {
        ErrorHandler::error(m_file_name, expr->m_right_brace.get_line(), "Empty lists are not allowed");
        return false;
    }

    // Find the type of the first element
    if (!type_check(expr->m_items.front().get())) {
        return false;
    }

    auto list_type = std::make_unique<type::List>(expr->m_items.front()->m_type->clone(), expr->m_items.size());

    for (int i = 1; i < expr->m_items.size(); i++) {
        auto item = expr->m_items[i].get();
        if (!type_check(item)) {
            return false;
        }

        // Make sure following elements have the same type as the first element
        if (!item->m_type->equals(list_type->m_elem_type.get())) {
            ErrorHandler::error(m_file_name, expr->m_right_brace.get_line(),
                std::format("Item at index {} is of type: {} but list is of type: {}",
                    i + 1, item->m_type->to_str(), list_type->m_elem_type->to_str())
                    .c_str());
            return false;
        }
    }

    expr->m_type = std::move(list_type);
    return true;
}

std::any jl::SemanticAnalyzer::visit_index_get_expr(IndexGet* expr)
{
    if (!type_check(expr->m_jlist.get()) || !type_check(expr->m_index_expr.get())) {
        // Component type check failed
        return false;
    } else if (expr->m_jlist.get()->m_type->m_kind != type::Type::LIST) {
        // Variable/Value not a list, so cannot be indexed
        ErrorHandler::error(m_file_name, expr->m_closing_bracket.get_line(),
            std::format("Cannot index into a value of type: {}", expr->m_jlist->m_type->to_str()).c_str());
        return false;
    } else if (!expr->m_index_expr->m_type->equals(&INT_CONSTANT)) {
        // Attempted to index with a non int value
        ErrorHandler::error(m_file_name, expr->m_closing_bracket.get_line(),
            std::format("Cannot index into a list with value of type: {}, only ints are allowed", expr->m_index_expr->m_type->to_str()).c_str());
        return false;
    } else {
        // All type checks are correct
        expr->m_type = static_cast<type::List*>(expr->m_jlist->m_type.get())->m_elem_type->clone();
        return true;
    }
}

std::any jl::SemanticAnalyzer::visit_index_set_expr(IndexSet* expr)
{
    if (!type_check(expr->m_jlist.get()) || !type_check(expr->m_index_expr.get()) || !type_check(expr->m_value_expr.get())) {
        // Component type check failed
        return false;
    } else if (expr->m_jlist.get()->m_type->m_kind != type::Type::LIST) {
        // Variable/Value not a list, so cannot be indexed
        ErrorHandler::error(m_file_name, expr->m_closing_bracket.get_line(),
            std::format("Cannot index into a value of type: {}", expr->m_jlist->m_type->to_str()).c_str());
        return false;
    } else if (!expr->m_index_expr->m_type->equals(&INT_CONSTANT)) {
        // Attempted to index with a non int value
        ErrorHandler::error(m_file_name, expr->m_closing_bracket.get_line(),
            std::format("Cannot index into a list with value of type: {}, only ints are allowed", expr->m_index_expr->m_type->to_str()).c_str());
        return false;
    } else if (!expr->m_value_expr->m_type->equals(dynamic_cast<type::List*>(expr->m_jlist->m_type.get())->m_elem_type.get())) {
        // LHS and RHS dont match
        ErrorHandler::error(m_file_name, expr->m_closing_bracket.get_line(),
            std::format("Cannot assign value of type: {} to list of type: {}",
                expr->m_value_expr->m_type->to_str(),
                expr->m_jlist->m_type->to_str())
                .c_str());
        return false;
    } else {
        // All type checks are correct
        expr->m_type = static_cast<type::List*>(expr->m_jlist->m_type.get())->m_elem_type->clone();
        return true;
    }
}

std::any jl::SemanticAnalyzer::visit_get_expr(Get* expr) { return false; }
std::any jl::SemanticAnalyzer::visit_set_expr(Set* expr) { return false; }
std::any jl::SemanticAnalyzer::visit_this_expr(This* expr) { return false; }
std::any jl::SemanticAnalyzer::visit_super_expr(Super* expr) { return false; }
std::any jl::SemanticAnalyzer::visit_type_cast_expr(TypeCast* expr) { return false; }

// -----------------------------------STMT---------------------------------

std::any jl::SemanticAnalyzer::visit_expr_stmt(ExprStmt* stmt)
{
    return type_check(stmt->m_expr.get());
}

std::any jl::SemanticAnalyzer::visit_var_stmt(VarStmt* stmt)
{
    std::optional<std::unique_ptr<type::Type>> final_type;

    if (stmt->m_initializer) {
        auto rhs_expr = stmt->m_initializer.value().get();
        if (!type_check(rhs_expr))
            return false;

        if (stmt->m_data_type) {
            auto type = type::from_type_info(*stmt->m_data_type);

            // Check whether the type exits
            if (!type) {
                ErrorHandler::error(m_file_name, stmt->m_name.get_line(),
                    std::format("Unrecognized type in var declaration: {}", stmt->m_data_type.value().name).c_str());
                return false;
            }

            // Handle array initializations
            if (type->get()->m_kind == type::Type::LIST && rhs_expr->m_type->m_kind == type::Type::LIST) {
                auto lhs = static_cast<type::List*>(type->get());
                auto rhs = static_cast<type::List*>(rhs_expr->m_type.get());

                // Nothing to do, everything is correct
                if (!lhs->m_elem_type->equals(rhs->m_elem_type.get())) {
                    goto error;
                }

                if (lhs->m_count < rhs->m_count) {
                    ErrorHandler::error(m_file_name, stmt->m_name.get_line(),
                        std::format("Array initializer has {} items which is more than the declared {} items", rhs->m_count, lhs->m_count).c_str());
                    return false;
                } else if (lhs->m_count > rhs->m_count) {
                    // Allocate more space
                    static_cast<JList*>(rhs_expr)->m_extra_item_count = lhs->m_count - rhs->m_count;
                }
            } else if (!type->get()->equals(rhs_expr->m_type.get())) {
                // Check whether the defined RHS type and parsed LHS types are the same for all non list types
            error:
                ErrorHandler::error(m_file_name, stmt->m_name.get_line(),
                    std::format("During var intialization variable {} of type {} cannot be assigned value of type {}",
                        stmt->m_name.get_lexeme(), type.value()->to_str(),
                        rhs_expr->m_type->to_str())
                        .c_str());
                return false;
            }
        }

        // If RHS is of type void
        if (rhs_expr->m_type->equals(&VOID_CONSTANT)) {
            ErrorHandler::error(m_file_name, stmt->m_name.get_line(), "Assignments involving void values are not allowed");
            return false;
        }

        final_type = rhs_expr->m_type->clone();
    } else {
        final_type = std::nullopt;
    }

    // If another variable of same name already exists in the current scope
    if (!define_variable(stmt->m_name.get_lexeme(), std::move(final_type))) {
        ErrorHandler::error(m_file_name, stmt->m_name.get_line(),
            std::format("Redefinition of variable {}", stmt->m_name.get_lexeme()).c_str());
        return false;
    }

    return true;
}

std::any jl::SemanticAnalyzer::visit_empty_stmt(EmptyStmt* stmt)
{
    return true;
}

std::any jl::SemanticAnalyzer::visit_block_stmt(BlockStmt* stmt)
{
    m_symbol_table.push_back({});
    const auto res = type_check(stmt->m_statements);
    m_symbol_table.pop_back();
    return res;
}

std::any jl::SemanticAnalyzer::visit_if_stmt(IfStmt* stmt)
{
    auto result = true;
    result &= type_check(stmt->m_condition.get());

    if (result && !type::is_boolean(stmt->m_condition->m_type.get())) {
        ErrorHandler::error(m_file_name, stmt->m_if_keyword.get_line(),
            std::format("Condition of if statement should be bool, but found {}", stmt->m_condition->m_type->to_str()).c_str());
        result = false;
    }

    result &= type_check(stmt->m_then_stmt.get());
    if (stmt->m_else_stmt) {
        result &= type_check(stmt->m_else_stmt.value().get());
    }

    return result;
}

std::any jl::SemanticAnalyzer::visit_while_stmt(WhileStmt* stmt)
{
    auto result = true;
    result &= type_check(stmt->m_condition.get());

    if (result && !type::is_boolean(stmt->m_condition->m_type.get())) {
        ErrorHandler::error(m_file_name, stmt->m_left_par.get_line(),
            std::format("Condition of while statement should be bool, but found {}", stmt->m_condition->m_type->to_str()).c_str());
        result = false;
    }

    result &= type_check(stmt->m_body.get());
    return result;
}

std::any jl::SemanticAnalyzer::visit_func_stmt(FuncStmt* stmt)
{
    auto result = true;
    std::vector<std::unique_ptr<type::Type>> param_types;
    // New block
    m_symbol_table.push_back({});

    for (int i = 0; i < stmt->m_data_types.size(); i++) {
        auto type = type::from_type_info(stmt->m_data_types[i]);

        if (!type) {
            ErrorHandler::error(m_file_name, stmt->m_params[i]->get_line(),
                std::format("Parameter {} in function {} has unrecognized type {} ",
                    stmt->m_params[i]->get_lexeme(), stmt->m_name.get_lexeme(), stmt->m_data_types[i].name)
                    .c_str());

            result = false;
            continue;
        }

        define_variable(stmt->m_params[i]->get_lexeme(), type->get()->clone());
        param_types.push_back(std::move(*type));
    }

    // Find the return type
    std::unique_ptr<type::Type> return_type;
    if (stmt->m_return_type) {
        auto type = type::from_type_info(*stmt->m_return_type);

        if (!type) {
            ErrorHandler::error(m_file_name, stmt->m_name.get_line(),
                std::format("Function {} has unrecognized return type {} ",
                    stmt->m_name.get_lexeme(), stmt->m_return_type->name)
                    .c_str());
            result = false;
        }

        return_type = std::move(*type);
    } else {
        return_type = std::make_unique<type::Builtin>(type::Builtin::VOID);
    }

    // Create the fucntion type
    auto func_type = std::make_unique<type::Func>(std::move(return_type), std::move(param_types));
    if (is_defined(stmt->m_name.get_lexeme())) {
        ErrorHandler::error(m_file_name, stmt->m_name.get_line(),
            std::format("Redefinition of variable {}", stmt->m_name.get_lexeme()).c_str());
        return false;
    }

    // Cache the func type in the stmt so that it could be refered in the code gen part (Dont know whether it will be useful)
    stmt->m_type = func_type->clone();
    // Store it in a stack to track inside which function we are currently in, needed for type checking return stmts
    m_func_types.push(func_type.get());
    // Define the function in the previous block
    m_symbol_table[m_symbol_table.size() - 2].insert({ stmt->m_name.get_lexeme(), std::move(func_type) });

    result = type_check(stmt->m_body);

    m_symbol_table.pop_back();
    m_func_types.pop();
    return result;
}

std::any jl::SemanticAnalyzer::visit_return_stmt(ReturnStmt* stmt)
{
    auto result = true;
    if (m_func_types.size() == 0) {
        ErrorHandler::error(m_file_name, stmt->m_keyword.get_line(), "Return can be used only inside functions");
        return false;
    }

    const type::Type* return_type;

    // Get the return type
    if (stmt->m_expr) {
        if (!type_check(stmt->m_expr->get()))
            return false;
        return_type = stmt->m_expr->get()->m_type.get();
    } else {
        return_type = &VOID_CONSTANT;
    }

    // Defined and parsed types differ
    auto ret_type = static_cast<const type::Func*>(m_func_types.top())->m_return_type.get();
    if (!ret_type->equals(return_type)) {
        ErrorHandler::error(m_file_name, stmt->m_keyword.get_line(),
            std::format("Expedted return type: {}, but found: {}",
                ret_type->to_str(), return_type->to_str())
                .c_str());
        return false;
    }

    return true;
}

std::any jl::SemanticAnalyzer::visit_print_stmt(PrintStmt* stmt)
{
    const auto res = type_check(stmt->m_expr.get());
    const auto kind = stmt->m_expr.get()->m_type->m_kind;

    if (res && kind != type::Type::BUILTIN && kind != type::Type::LIST) {
        ErrorHandler::error(
            m_file_name, stmt->m_token.get_line(),
            std::format("Only primitives and lists can be printed, but found type: {}",
                stmt->m_expr.get()->m_type.get()->to_str())
                .c_str());
        return false;
    }

    if (kind == type::Type::LIST) {
        const auto list = dynamic_cast<type::List*>(stmt->m_expr->m_type.get());
        if (list->m_elem_type->m_kind != type::Type::BUILTIN) {
            ErrorHandler::error(
                m_file_name, stmt->m_token.get_line(),
                std::format("Expected a list of primitives for printing but fount", list->to_str()).c_str());
            return false;
        }
    }

    return res;
}

std::any jl::SemanticAnalyzer::visit_class_stmt(ClassStmt* stmt) { return false; }
std::any jl::SemanticAnalyzer::visit_for_each_stmt(ForEachStmt* stmt) { return false; }
std::any jl::SemanticAnalyzer::visit_break_stmt(BreakStmt* stmt) { return false; }
std::any jl::SemanticAnalyzer::visit_extern_stmt(ExternStmt* stmt) { return false; }

//--------------------------------------------------------------------------------------------------

bool are_pointers(jl::type::Type* left, jl::type::Type* right)
{
    if (left->m_kind == jl::type::Type::PTR || right->m_kind == jl::type::Type::PTR) {
        return true;
    } else {
        return false;
    }
}

bool validate_ptr_arithmetic(jl::Binary* expr, const jl::type::Type* left, const jl::type::Type* right, std::string m_file_name)
{
    using namespace jl;
    unimplemented("Incomplete Pointer arithametics");
    // Typecast the non pointer to a pointer
    if (left->m_kind != type::Type::PTR) {
        if (left->m_kind != type::Type::BUILTIN) {
            ErrorHandler::error(
                m_file_name,
                expr->m_oper.get_line(),
                std::format("Operation: {} is not permitted for Right: {} (is a pointer) but left: {} (not a pointer)",
                    expr->m_oper.get_lexeme(),
                    right->to_str(),
                    left->to_str())
                    .c_str());
            return false;
        } else if (static_cast<const type::Builtin*>(left)->m_primitive != type::Builtin::INT) {
            ErrorHandler::error(
                m_file_name,
                expr->m_oper.get_line(),
                std::format("Operation: {} is not permitted for Right: {} (is a pointer) but left: {} (not a int)",
                    expr->m_oper.get_lexeme(),
                    right->to_str(),
                    left->to_str())
                    .c_str());
            return false;
        } else {
            // expr->m_left->m_cast_to = std::make_unique<type::Pointer>(static_cast<type::Pointer>(right).m_pointee);
            // TODO::Create a copy method for Type
        }
    }

    if (right->m_kind != type::Type::PTR) {
        if (right->m_kind != type::Type::BUILTIN) {
            ErrorHandler::error(
                m_file_name,
                expr->m_oper.get_line(),
                std::format("Operation: {} is not permitted for left: {} (is a pointer) and right: {} (not a pointer)",
                    expr->m_oper.get_lexeme(),
                    left->to_str(),
                    right->to_str())
                    .c_str());
            return false;
        } else if (static_cast<const type::Builtin*>(right)->m_primitive != type::Builtin::INT) {
            ErrorHandler::error(
                m_file_name,
                expr->m_oper.get_line(),
                std::format("Operation: {} is not permitted for left: {} (is a pointer) and right: {} (not a int)",
                    expr->m_oper.get_lexeme(),
                    left->to_str(),
                    right->to_str())
                    .c_str());
            return false;
        } else {
            // expr->m_left->m_cast_to = std::make_unique<type::Pointer>(static_cast<type::Pointer>(right_type).m_pointee);
        }

        // expr->m_type =
    }

    return true;
}

std::pair<jl::type::Builtin::Primitive, jl::type::Builtin::Primitive> apply_numeric_promotion(
    jl::Binary* expr,
    jl::type::Builtin* left,
    jl::type::Builtin* right)
{
    using namespace jl;

    auto l_type = left->m_primitive;
    auto r_type = right->m_primitive;

    if (l_type == type::Builtin::FLOAT && r_type == type::Builtin::INT) {
        expr->m_right->m_cast_to = std::make_unique<type::Builtin>(type::Builtin::FLOAT);
        r_type = type::Builtin::FLOAT;
    } else if (left->m_primitive == type::Builtin::INT && r_type == type::Builtin::FLOAT) {
        expr->m_left->m_cast_to = std::make_unique<type::Builtin>(type::Builtin::FLOAT);
        l_type = type::Builtin::FLOAT;
    }

    return { l_type, r_type };
}

bool is_float_allowed_op(jl::Token::TokenType t)
{
    using namespace jl;
    switch (t) {
    case Token::PLUS:
    case Token::MINUS:
    case Token::STAR:
    case Token::SLASH:
    case Token::GREATER:
    case Token::LESS:
    case Token::GREATER_EQUAL:
    case Token::LESS_EQUAL:
    case Token::EQUAL_EQUAL:
    case Token::BANG_EQUAL:
        return true;
    case Token::PERCENT:
    case Token::BIT_AND:
    case Token::BIT_OR:
    case Token::BIT_XOR:
    default:
        return false;
    }
}

bool is_boolean_operator(jl::Token::TokenType t)
{
    using namespace jl;
    switch (t) {
    case Token::PLUS:
    case Token::MINUS:
    case Token::STAR:
    case Token::SLASH:
    case Token::PERCENT:
    case Token::BIT_AND:
    case Token::BIT_OR:
    case Token::BIT_XOR:
        return false;
    case Token::GREATER:
    case Token::LESS:
    case Token::GREATER_EQUAL:
    case Token::LESS_EQUAL:
    case Token::EQUAL_EQUAL:
    case Token::BANG_EQUAL:
    default:
        return true;
    }
}
