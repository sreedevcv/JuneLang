#include "CodeGenerator.hpp"

#include "Chunk.hpp"
#include "DataSection.hpp"
#include "ErrorHandler.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include "Value.hpp"

#include <any>
#include <cstdint>
#include <iostream>
#include <optional>
#include <print>
#include <string>
#include <utility>
#include <vector>

jl::CodeGenerator::CodeGenerator(std::string& file_name)
    : m_file_name(file_name)
{
    Chunk chunk { "__root__" };
    push_chunk(std::move(chunk), "__root__");
}

jl::CodeGenerator::~CodeGenerator()
{
}

const std::pair<std::map<std::string, jl::Chunk>&, jl::DataSection&>
jl::CodeGenerator::generate(std::vector<Stmt*> stmts)
{
    for (auto stmt : stmts) {
        compile(stmt);
    }

    m_chunk->write_control(OpCode::RETURN, Nil {}, m_chunk->get_last_line());
    return { m_chunk_list, data_section };
}

const jl::Chunk& jl::CodeGenerator::get_root_chunk() const
{
    return m_chunk_list.at("__root__");
}

jl::TempVar jl::CodeGenerator::compile(Stmt* stmt)
{
    return std::any_cast<TempVar>(stmt->accept(*this));
}

jl::TempVar jl::CodeGenerator::compile(Expr* expr)
{
    return std::any_cast<TempVar>((expr->accept(*this)));
}

void jl::CodeGenerator::disassemble()
{
    for (const auto& [name, chunk] : m_chunk_list) {
        std::println("{}", chunk.disassemble());
    }

    std::println("Memory:");
    data_section.disassemble(std::cout);
}

jl::TempVar jl::CodeGenerator::empty_var()
{
    return m_chunk->create_temp_var(OperandType::NIL);
}

bool jl::CodeGenerator::check_if_func_exists(const std::string& name) const
{
    return m_chunk_list.contains(name);
}

void jl::CodeGenerator::push_chunk(Chunk&& chunk, const std::string& name)
{
    m_chunk_list.insert({ name, std::move(chunk) });
    m_func_stack.push(&m_chunk_list.at(name));
    m_chunk = m_func_stack.top();
}

void jl::CodeGenerator::pop_chunk()
{
    m_func_stack.pop();
    m_chunk = m_func_stack.top();
}

/* ======================================================================================================================== */
/* -----------------------------------------------------EXPR--------------------------------------------------------------- */
/* ======================================================================================================================== */

std::any jl::CodeGenerator::visit_binary_expr(Binary* expr)
{
    const auto type = expr->m_oper->get_tokentype();
    const auto l = compile(expr->m_left);
    const auto r = compile(expr->m_right);
    const uint32_t line = expr->m_oper->get_line();

    TempVar dest_var;

    switch (type) {
    case Token::PLUS: {
        dest_var = m_chunk->write(OpCode::ADD, l, r, line);
    } break;
    case Token::MINUS: {
        dest_var = m_chunk->write(OpCode::MINUS, l, r, line);
    } break;
    case Token::STAR: {
        dest_var = m_chunk->write(OpCode::STAR, l, r, line);
    } break;
    case Token::SLASH: {
        dest_var = m_chunk->write(OpCode::SLASH, l, r, line);
    } break;
    case Token::GREATER: {
        dest_var = m_chunk->write(OpCode::GREATER, l, r, line);
    } break;
    case Token::LESS: {
        dest_var = m_chunk->write(OpCode::LESS, l, r, line);
    } break;
    case Token::GREATER_EQUAL: {
        dest_var = m_chunk->write(OpCode::GREATER_EQUAL, l, r, line);
    } break;
    case Token::LESS_EQUAL: {
        dest_var = m_chunk->write(OpCode::LESS_EQUAL, l, r, line);
    } break;
    case Token::PERCENT: {
        dest_var = m_chunk->write(OpCode::MODULUS, l, r, line);
    } break;
    case Token::EQUAL_EQUAL: {
        dest_var = m_chunk->write(OpCode::EQUAL, l, r, line);
    } break;
    case Token::BANG_EQUAL: {
        dest_var = m_chunk->write(OpCode::NOT_EQUAL, l, r, line);
    } break;
    default:
        unimplemented();
        break;
    }

    return dest_var;
}

std::any jl::CodeGenerator::visit_grouping_expr(Grouping* expr)
{
    return compile(expr->m_expr);
}

std::any jl::CodeGenerator::visit_unary_expr(Unary* expr)
{
    auto val = compile(expr->m_expr);
    auto oper = expr->m_oper->get_tokentype();
    TempVar temp;

    switch (oper) {
    case Token::MINUS:
        temp = m_chunk->write(OpCode::MINUS, val, expr->m_oper->get_line());
        break;
    case Token::BANG:
        temp = m_chunk->write(OpCode::NOT, val, expr->m_oper->get_line());
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
    case Type::CHAR: {
        literal = std::get<char>(expr->m_value->get());
    } break;
    case Type::STR: {
        const auto& str = std::get<std::string>(expr->m_value->get());
        literal = m_chunk->add_data(data_section, str, OperandType::CHAR_PTR);
    } break;
    default:
        unimplemented();
        break;
    }

    TempVar var = m_chunk->write(OpCode::MOVE, literal, m_chunk->get_last_line());

    return var;
}

std::any jl::CodeGenerator::visit_logical_expr(Logical* expr)
{
    const auto l = compile(expr->m_left);
    const auto r = compile(expr->m_right);
    const auto oper = expr->m_oper.get_tokentype();
    TempVar temp;

    if (oper == Token::OR) {
        temp = m_chunk->write(OpCode::OR, l, r, expr->m_oper.get_line());
    } else {
        temp = m_chunk->write(OpCode::AND, l, r, expr->m_oper.get_line());
    }

    return temp;
}

// Retreive the temp_var associated with variable
// It should have already been seen by visit_var_stmt
std::any jl::CodeGenerator::visit_variable_expr(Variable* expr)
{
    const auto& var_name = expr->m_name.get_lexeme();
    const auto temp_var = m_chunk->look_up_variable(var_name);

    if (temp_var) {
        return *temp_var;
    }

    // Variable does not exist in this chunk
    // Look for it in the __root__ chunk
    const auto root_var = m_chunk_list.at("__root__").look_up_variable(var_name);

    if (!root_var) {
        ErrorHandler::error(m_file_name, expr->m_name.get_line(), "Use of unknow variable");
        return empty_var();
    }

    // Currently only functions are available from the __root__ scope
    // If the temp_var is a function in the __root__ scope, then make it
    // available in the current function scope

    // Ensure that its a fucntion
    std::optional<std::string> func_name;
    for (const auto& [temp, name] : m_chunk_list.at("__root__").m_registered_functions) {
        if (temp == (*root_var).idx) {
            func_name = name;
        }
    }

    if (!func_name || !check_if_func_exists(*func_name)) {
        ErrorHandler::error(m_file_name, expr->m_name.get_line(), "Use of unknow variable");
        return empty_var();
    }

    // Register this as a local named variable
    const auto func_var = m_chunk->store_variable(*func_name, OperandType::TEMP);
    m_chunk->register_function(func_var->idx, *func_name);

    if (!func_var) {
        ErrorHandler::error(m_file_name, expr->m_name.get_line(), "Local variable same as a function name");
        return empty_var();
    }
    return *func_var;
}

std::any jl::CodeGenerator::visit_assign_expr(Assign* expr)
{
    const auto assignee = compile(expr->m_expr);

    const auto& var_name = expr->m_token.get_lexeme();
    const auto dest_var = m_chunk->look_up_variable(var_name);

    if (dest_var) {
        m_chunk->write_with_dest(OpCode::MOVE, assignee, *dest_var, expr->m_token.get_line());
    } else {
        unimplemented();
    }

    // For now return the dest_var
    // Not sure whether the return value will be used
    return *dest_var;
}

std::any jl::CodeGenerator::visit_call_expr(Call* expr)
{
    // Compile the callee
    const auto func_temp_var = compile(expr->m_callee);
    const auto line = expr->m_paren.get_line();

    // Ensure that its a temp_var
    if (get_type(func_temp_var) != OperandType::TEMP) {
        ErrorHandler::error(m_file_name, line, "Only functions can be called");
        return empty_var();
    }

    // Ensure temp-var is associated with a named function
    std::optional<std::string> func_name;
    for (const auto& [temp, name] : m_chunk->m_registered_functions) {
        if (temp == (func_temp_var).idx) {
            func_name = name;
        }
    }

    if (!func_name || !check_if_func_exists(*func_name)) {
        ErrorHandler::error(m_file_name, line, "No such function exists");
        return empty_var();
    }

    // Get the function chunk
    const auto& func_chunk = m_chunk_list.at(*func_name);
    const auto& func_inputs = func_chunk.get_input_variable_names();

    // Ensure whether the arity is same
    if (func_inputs.size() != expr->m_arguments.size()) {
        ErrorHandler::error(m_file_name, line, "No. of func arguments is wrong");
        return empty_var();
    }

    std::vector<TempVar> args;

    // Compile all the function arguments
    for (int i = 0; i < expr->m_arguments.size(); i++) {
        auto arg = expr->m_arguments[i];
        const auto arg_var = compile(arg);

        // The data type should be same as function signature
        const auto& arg_name = func_inputs[i];
        const auto temp = *func_chunk.look_up_variable(arg_name);

        const auto expected_type = func_chunk.get_nested_type(temp);
        const auto actual_type = m_chunk->get_nested_type(arg_var);

        if (expected_type != actual_type) {
            ErrorHandler::error(
                m_file_name,
                line,
                std::format("Func argument {} type mismatch. Expected {} found {}",
                    i,
                    to_string(expected_type),
                    to_string(actual_type))
                    .c_str());
        }

        args.push_back(arg_var);
    }

    const auto ret_var = m_chunk->create_temp_var(func_chunk.return_type);
    m_chunk->write_call(
        OpCode::CALL,
        func_temp_var,
        *func_name,
        ret_var,
        std::move(args),
        func_chunk.extern_symbol,
        m_chunk->get_last_line());

    return ret_var;
}

std::any jl::CodeGenerator::visit_index_get_expr(IndexGet* expr)
{
    const auto list_ptr = compile(expr->m_jlist);
    const auto idx = compile(expr->m_index_expr);
    const auto list_ptr_type = m_chunk->get_nested_type(list_ptr);

    if (!is_pure_ptr(list_ptr_type)) {
        ErrorHandler::error(
            m_file_name,
            expr->m_closing_bracket.get_line(),
            "Attempting to index a non pointer!");
        return empty_var();
    }

    if (m_chunk->get_nested_type(idx) != OperandType::INT) {
        ErrorHandler::error(
            m_file_name,
            expr->m_closing_bracket.get_line(),
            "Attempting to index a pointer with a non-integer value!");
        return empty_var();
    }

    // Calculate relative address
    const auto addr = m_chunk->write(OpCode::ADD, list_ptr, idx, m_chunk->get_last_line());
    const auto result_var = m_chunk->create_temp_var(*from_ptr(list_ptr_type));
    m_chunk->write_with_dest(OpCode::LOAD, addr, result_var, m_chunk->get_last_line());
    return result_var;
}

// Shouldn't this be a stmt?
std::any jl::CodeGenerator::visit_index_set_expr(IndexSet* expr)
{
    const auto list_ptr = compile(expr->m_jlist);
    const auto idx = compile(expr->m_index_expr);
    const auto new_value = compile(expr->m_value_expr);
    const auto list_ptr_type = m_chunk->get_nested_type(list_ptr);

    const auto idx_type = m_chunk->get_nested_type(idx);
    const auto value_type = m_chunk->get_nested_type(new_value);

    if (!is_pure_ptr(list_ptr_type)) {
        ErrorHandler::error(
            m_file_name,
            expr->m_closing_bracket.get_line(),
            "Attempting to index a non pointer!");
        return empty_var();
    }

    if (idx_type != OperandType::INT) {
        ErrorHandler::error(
            m_file_name,
            expr->m_closing_bracket.get_line(),
            "Attempting to index a pointer with a non-integer value!");
        return empty_var();
    }

    if (value_type != *from_ptr(list_ptr_type)) {
        ErrorHandler::error(
            m_file_name,
            expr->m_closing_bracket.get_line(),
            std::format("Cannot write a value of type {} to a pointer of type {} !",
                to_string(value_type),
                to_string(list_ptr_type))
                .c_str());
        return empty_var();
    }

    // Calculate relative address
    const auto addr = m_chunk->write(OpCode::ADD, list_ptr, idx, m_chunk->get_last_line());
    m_chunk->write_jump_or_store(OpCode::STORE, new_value, addr, m_chunk->get_last_line());
    return addr;
}

std::any jl::CodeGenerator::visit_jlist_expr(JList* expr)
{
    // Determine type of list
    OperandType list_type;
    if (expr->m_items.size() == 0) {
        list_type = OperandType::UNASSIGNED;
        unimplemented("Zero element arrays are not supported right now");
    } else {

        const auto first_ele = compile(expr->m_items[0]);
        const auto first_ele_type = m_chunk->get_nested_type(first_ele);
        const auto ptr_type = into_ptr(first_ele_type);
        const auto list_start_offset = data_section.add_data(size_of_type(first_ele_type));

        if (!ptr_type) {
            ErrorHandler::error(m_file_name, m_chunk->get_last_line(), "Unsupported type(No ptr for this type available)");
            return first_ele;
        }

        const auto list_var = m_chunk->create_ptr_var(*ptr_type, list_start_offset);

        m_chunk->write_jump_or_store(
            OpCode::STORE,
            first_ele,
            list_var,
            m_chunk->get_last_line());

        list_type = m_chunk->get_nested_type(first_ele);

        for (int i = 1; i < expr->m_items.size(); i++) {
            const auto ele = compile(expr->m_items[i]);

            if (m_chunk->get_nested_type(ele) != list_type) {
                ErrorHandler::error(m_file_name, m_chunk->get_last_line(), "List contains elements of different types");
                continue;
            }

            const auto offset = data_section.add_data(size_of_type(first_ele_type));
            const auto ele_ptr = m_chunk->create_ptr_var(*ptr_type, offset);

            m_chunk->write_jump_or_store(
                OpCode::STORE,
                ele,
                ele_ptr,
                m_chunk->get_last_line());
        }

        return list_var;
    }

    return empty_var();
}

std::any jl::CodeGenerator::visit_get_expr(Get* expr) { }
std::any jl::CodeGenerator::visit_set_expr(Set* expr) { }
std::any jl::CodeGenerator::visit_this_expr(This* expr) { }
std::any jl::CodeGenerator::visit_super_expr(Super* expr) { }

/* ======================================================================================================================== */
/* -----------------------------------------------------STMT--------------------------------------------------------------- */
/* ======================================================================================================================== */

/* Statements are meant to return nothing, they are self contained */

std::any jl::CodeGenerator::visit_var_stmt(VarStmt* stmt)
{
    Operand operand;
    if (stmt->m_initializer != nullptr) {
        operand = compile(stmt->m_initializer);
    }

    OperandType type_name = OperandType::UNASSIGNED;
    if (stmt->m_data_type) {
        const auto type = from_typeinfo(*(stmt->m_data_type));
        if (type) {
            type_name = *type;
        } else {
            ErrorHandler::error(m_file_name, stmt->m_name.get_line(), "Unknown data type");
        }
    }

    auto var = m_chunk->store_variable(stmt->m_name.get_lexeme(), type_name);
    if (!var) {
        ErrorHandler::error(m_file_name, stmt->m_name.get_line(), "Redeclaration of a variable");
        return empty_var();
    }

    if (stmt->m_initializer != nullptr) {
        m_chunk->write_with_dest(OpCode::MOVE, operand, *var, stmt->m_name.get_line());
    }

    return *var;
}

std::any jl::CodeGenerator::visit_expr_stmt(ExprStmt* stmt)
{
    const auto operand = compile(stmt->m_expr);
    return operand;
}

std::any jl::CodeGenerator::visit_block_stmt(BlockStmt* stmt)
{
    m_chunk->push_block();

    for (auto s : stmt->m_statements) {
        compile(s);
    }

    m_chunk->pop_block();

    return empty_var();
}

std::any jl::CodeGenerator::visit_while_stmt(WhileStmt* stmt)
{
    const int32_t loop_start_label = m_chunk->create_new_label();
    const int32_t loop_end_label = m_chunk->create_new_label();
    // Write the label to jmp to start the loop again
    m_chunk->write_control(OpCode::LABEL, loop_start_label, m_chunk->get_last_line());

    const auto condition = compile(stmt->m_condition);

    if (get_type(condition) != OperandType::BOOL
        && m_chunk->get_nested_type(condition) != OperandType::BOOL) {
        ErrorHandler::error(
            m_file_name,
            m_chunk->get_last_line(),
            "While loop condition doesn't evaluate to a bool!");
    }
    // Jmp to end if condition false
    m_chunk->write_jump_or_store(
        OpCode::JMP_UNLESS,
        condition,
        loop_end_label,
        m_chunk->get_last_line());

    compile(stmt->m_body);

    // Jmp to beginning
    m_chunk->write_control(
        OpCode::JMP,
        loop_start_label,
        m_chunk->get_last_line());

    // Write the ending label
    m_chunk->write_control(
        OpCode::LABEL,
        loop_end_label,
        m_chunk->get_last_line());

    return empty_var();
}
std::any jl::CodeGenerator::visit_empty_stmt(EmptyStmt* stmt)
{
    return empty_var();
}

std::any jl::CodeGenerator::visit_if_stmt(IfStmt* stmt)
{
    const auto else_label = m_chunk->create_new_label();
    const auto condition = compile(stmt->m_condition);
    // Jump to after if-block if condition fails
    m_chunk->write_jump_or_store(OpCode::JMP_UNLESS, condition, else_label, m_chunk->get_last_line());
    // Compile if-block
    compile(stmt->m_then_stmt);

    if (stmt->m_else_stmt != nullptr) {
        // Jmp to the end of if-else ladder if contol comes after execution of if-block
        const auto end_label = m_chunk->create_new_label();
        m_chunk->write_control(OpCode::JMP, end_label, m_chunk->get_last_line());

        // Place the else label to come if the previous condtion fails
        m_chunk->write_control(OpCode::LABEL, else_label, m_chunk->get_last_line());
        // Compile the else body
        compile(stmt->m_else_stmt);
        // Place the ending label
        m_chunk->write_control(OpCode::LABEL, end_label, m_chunk->get_last_line());
    } else {
        m_chunk->write_control(OpCode::LABEL, else_label, m_chunk->get_last_line());
    }

    return empty_var();
}

std::any jl::CodeGenerator::visit_func_stmt(FuncStmt* stmt)
{
    if (check_if_func_exists(stmt->m_name.get_lexeme())) {
        ErrorHandler::error(
            m_file_name,
            stmt->m_name.get_line(),
            "Functions with the same name!");
        return empty_var();
    }

    // Retrieve return type
    OperandType return_type;
    if (stmt->m_return_type == nullptr) {
        return_type = OperandType::NIL;
    } else {
        const auto type = from_str(stmt->m_return_type->get_lexeme());
        if (type) {
            return_type = *type;
        } else {
            return_type = OperandType::NIL;
            ErrorHandler::error(
                m_file_name,
                stmt->m_return_type->get_line(),
                "Function has unkown return type!");
        }
    }

    // Store the function_name as a variable in the outer chunk
    const auto outer_var = m_chunk->store_variable(stmt->m_name.get_lexeme(), return_type);
    m_chunk->register_function(outer_var->idx, stmt->m_name.get_lexeme());

    Chunk chunk { stmt->m_name.get_lexeme() };
    push_chunk(std::move(chunk), stmt->m_name.get_lexeme());

    // Store the function_name as a variable in the new chunk also for recursion support
    const auto func_var = m_chunk->store_variable(stmt->m_name.get_lexeme(), return_type);
    m_chunk->register_function(func_var->idx, stmt->m_name.get_lexeme());

    // Store the return type
    m_chunk->return_type = return_type;

    std::vector<TempVar> parameter_vars;

    // Add function parameters to chunk
    for (int i = 0; i < stmt->m_params.size(); i++) {
        const auto type = from_typeinfo(stmt->m_data_types[i]);
        auto param_type = OperandType::UNASSIGNED;

        if (type) {
            param_type = *type;
        } else {
            ErrorHandler::error(m_file_name, stmt->m_params[i]->get_line(), "Unknown data type");
        }

        const auto var = m_chunk->add_input_parameter(stmt->m_params[i]->get_lexeme(), param_type);
        parameter_vars.push_back(var);
    }

    if (stmt->is_extern) {
        // Extern function declaration
    } else {
        // Normal june function
        // Compile the function body
        for (auto s : stmt->m_body) {
            compile(s);
        }

        // Write a return statement if the last ir written is not a return
        m_chunk->write_control(OpCode::RETURN, default_operand(return_type), m_chunk->get_last_line());
    }

    pop_chunk();

    return empty_var();
}

std::any jl::CodeGenerator::visit_return_stmt(ReturnStmt* stmt)
{
    const auto line = stmt->m_keyword.get_line();

    // Can return but is not returning anything
    if (stmt->m_expr == nullptr && m_chunk->return_type != OperandType::NIL) {
        ErrorHandler::error(m_file_name, line, "Non-void function is not returning a value");
        m_chunk->write_control(OpCode::RETURN, Nil {}, line);
        return empty_var();
    }

    // Cannot return a value but is still returning something
    if (stmt->m_expr != nullptr && m_chunk->return_type == OperandType::NIL) {
        ErrorHandler::error(m_file_name, line, "Void function cannot return a value");
        return empty_var();
    }

    if (stmt->m_expr != nullptr) {
        const auto ret_var = compile(stmt->m_expr);

        // Types don't match
        if (m_chunk->get_nested_type(ret_var) != m_chunk->return_type) {
            ErrorHandler::error(m_file_name, line, "Return value of function does not match declaration");
        }

        // m_chunk->write_control(OpCode::PUSH, ret_var, m_chunk->get_last_line());
        m_chunk->write_control(OpCode::RETURN, ret_var, m_chunk->get_last_line());
    } else {

        m_chunk->write_control(OpCode::RETURN, Nil {}, m_chunk->get_last_line());
    }

    return empty_var();
}

std::any jl::CodeGenerator::visit_extern_stmt(ExternStmt* stmt)
{
    compile(stmt->m_june_func);
    const auto& symbol_name = std::get<std::string>(stmt->m_symbol_name.get_value()->get());
    // Get the just compiled function
    auto& chunk = m_chunk_list.at(stmt->m_june_func->m_name.get_lexeme());
    chunk.extern_symbol = symbol_name;
    return empty_var();
}

std::any jl::CodeGenerator::visit_print_stmt(PrintStmt* stmt) { }
std::any jl::CodeGenerator::visit_class_stmt(ClassStmt* stmt) { }
std::any jl::CodeGenerator::visit_for_each_stmt(ForEachStmt* stmt) { }
std::any jl::CodeGenerator::visit_break_stmt(BreakStmt* stmt) { }