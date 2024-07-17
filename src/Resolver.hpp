#pragma once

#include <map>

#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Stmt.hpp"

namespace jl {

class Resolver : public IExprVisitor, public IStmtVisitor {
public:
    Resolver(Interpreter& interpreter, std::string& file_name);
    ~Resolver() = default;

    void resolve(std::vector<Stmt*>& statements);

    // NOTE::Change this to a enum class for uniformity
    enum FunctionType {
        NONE,
        FUNCTION,
        INITIALIZER,
        METHOD,
    };

    enum class ClassType {
        NONE,
        CLASS,
        SUBCLASS,
    };

private:
    using Scope = std::map<std::string, bool>;

    Interpreter& m_interpreter;
    std::vector<Scope> m_scopes;
    std::string& m_file_name;
    FunctionType m_current_function_type = NONE;
    ClassType m_current_class_type = ClassType::NONE;

    void resolve(Stmt* statement);
    void resolve(Expr* expression);
    void resolve_local(Expr* expr, Token& name);
    void resolve_function(FuncStmt* stmt, FunctionType function_type);
    void begin_scope();
    void end_scope();
    void declare(Token& name);
    void define(Token& name);

    virtual std::any visit_assign_expr(Assign* expr) override;
    virtual std::any visit_binary_expr(Binary* expr) override;
    virtual std::any visit_grouping_expr(Grouping* expr) override;
    virtual std::any visit_unary_expr(Unary* expr) override;
    virtual std::any visit_literal_expr(Literal* expr) override;
    virtual std::any visit_variable_expr(Variable* expr) override;
    virtual std::any visit_logical_expr(Logical* expr) override;
    virtual std::any visit_call_expr(Call* expr) override;
    virtual std::any visit_get_expr(Get* expr) override;
    virtual std::any visit_set_expr(Set* expr) override;
    virtual std::any visit_this_expr(This* expr) override;
    virtual std::any visit_super_expr(Super* expr) override;
    virtual std::any visit_jlist_expr(JList* expr) override;
    virtual std::any visit_index_get_expr(IndexGet* expr) override;
    virtual std::any visit_index_set_expr(IndexSet* expr) override;


    virtual std::any visit_print_stmt(PrintStmt* stmt) override;
    virtual std::any visit_expr_stmt(ExprStmt* stmt) override;
    virtual std::any visit_var_stmt(VarStmt* stmt) override;
    virtual std::any visit_block_stmt(BlockStmt* stmt) override;
    virtual std::any visit_empty_stmt(EmptyStmt* stmt) override;
    virtual std::any visit_if_stmt(IfStmt* stmt) override;
    virtual std::any visit_while_stmt(WhileStmt* stmt) override;
    virtual std::any visit_func_stmt(FuncStmt* stmt) override;
    virtual std::any visit_return_stmt(ReturnStmt* stmt) override;
    virtual std::any visit_class_stmt(ClassStmt* stmt) override;
    virtual std::any visit_for_each_stmt(ForEachStmt* stmt) override;
};

} // namespace jl
