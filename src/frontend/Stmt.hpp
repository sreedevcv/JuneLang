#pragma once

#include "Expr.hpp"
#include "Token.hpp"
#include "TypeInfo.hpp"

#include <optional>
#include <utility>
#include <vector>

namespace jl {

class PrintStmt;
class ExprStmt;
class VarStmt;
class BlockStmt;
class EmptyStmt;
class IfStmt;
class WhileStmt;
class FuncStmt;
class ReturnStmt;
class ClassStmt;
class ForEachStmt;
class BreakStmt;
class ExternStmt;

class IStmtVisitor {
public:
    virtual std::any visit_print_stmt(PrintStmt* stmt) = 0;
    virtual std::any visit_expr_stmt(ExprStmt* stmt) = 0;
    virtual std::any visit_var_stmt(VarStmt* stmt) = 0;
    virtual std::any visit_block_stmt(BlockStmt* stmt) = 0;
    virtual std::any visit_empty_stmt(EmptyStmt* stmt) = 0;
    virtual std::any visit_if_stmt(IfStmt* stmt) = 0;
    virtual std::any visit_while_stmt(WhileStmt* stmt) = 0;
    virtual std::any visit_func_stmt(FuncStmt* stmt) = 0;
    virtual std::any visit_return_stmt(ReturnStmt* stmt) = 0;
    virtual std::any visit_class_stmt(ClassStmt* stmt) = 0;
    virtual std::any visit_for_each_stmt(ForEachStmt* stmt) = 0;
    virtual std::any visit_break_stmt(BreakStmt* stmt) = 0;
    virtual std::any visit_extern_stmt(ExternStmt* stmt) = 0;
};

class Stmt {
public:
    virtual std::any accept(IStmtVisitor& visitor) = 0;
    virtual ~Stmt() = default;
};

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> m_expr;

    inline ExprStmt(std::unique_ptr<Expr> expr)
        : m_expr(std::move(expr))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_expr_stmt(this);
    }

    virtual ~ExprStmt() = default;
};

class PrintStmt : public Stmt {
public:
    std::unique_ptr<Expr> m_expr;

    inline PrintStmt(std::unique_ptr<Expr> expr)
        : m_expr(std::move(expr))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_print_stmt(this);
    }

    virtual ~PrintStmt() = default;
};

class VarStmt : public Stmt {
public:
    Token m_name;
    std::optional<std::unique_ptr<Expr>> m_initializer;
    std::optional<TypeInfo> m_data_type;

    inline VarStmt(Token& name, std::optional<std::unique_ptr<Expr>> initializer, std::optional<TypeInfo> data_type)
        : m_name(std::move(name))
        , m_initializer(std::move(initializer))
        , m_data_type(std::move(data_type))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_var_stmt(this);
    }

    virtual ~VarStmt() = default;
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> m_statements;

    inline BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : m_statements(std::move(statements))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_block_stmt(this);
    }

    virtual ~BlockStmt() = default;
};

class EmptyStmt : public Stmt {
public:
    EmptyStmt() = default;

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_empty_stmt(this);
    }

    virtual ~EmptyStmt() = default;
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> m_condition;
    std::unique_ptr<Stmt> m_then_stmt;
    std::optional<std::unique_ptr<Stmt>> m_else_stmt;

    inline IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_stmt, std::optional<std::unique_ptr<Stmt>> else_stmt)
        : m_condition(std::move(condition))
        , m_then_stmt(std::move(then_stmt))
        , m_else_stmt(std::move(else_stmt))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_if_stmt(this);
    }

    virtual ~IfStmt() = default;
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> m_condition;
    std::unique_ptr<Stmt> m_body;

    inline WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : m_condition(std::move(condition))
        , m_body(std::move(body))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_while_stmt(this);
    }

    virtual ~WhileStmt() = default;
};

class FuncStmt : public Stmt {
public:
    Token m_name;
    std::vector<Token*> m_params; // TODO::Should i delete these??
    std::vector<TypeInfo> m_data_types;
    std::optional<TypeInfo> m_return_type;
    std::vector<std::unique_ptr<Stmt>> m_body;
    bool is_extern;

    inline FuncStmt(
        Token& name,
        std::vector<Token*>& params,
        std::vector<TypeInfo> data_types,
        std::optional<TypeInfo> return_type,
        std::vector<std::unique_ptr<Stmt>> body)
        : m_name(std::move(name))
        , m_params(params)
        , m_data_types(std::move(data_types))
        , m_return_type(return_type)
        , m_body(std::move(body))
    {
        is_extern = false;
    }

    inline FuncStmt(
        Token name,
        std::vector<Token*>& params,
        std::vector<TypeInfo> data_types,
        std::optional<TypeInfo> return_type)
        : m_name(std::move(name))
        , m_params(params)
        , m_data_types(std::move(data_types))
        , m_return_type(return_type)
    {
        is_extern = true;
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_func_stmt(this);
    }

    virtual ~FuncStmt() = default;
};

class ReturnStmt : public Stmt {
public:
    Token m_keyword;
    std::optional<std::unique_ptr<Expr>> m_expr;

    inline ReturnStmt(Token& keyword, std::optional<std::unique_ptr<Expr>> expr)
        : m_keyword(std::move(keyword))
        , m_expr(std::move(expr))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_return_stmt(this);
    }

    virtual ~ReturnStmt() = default;
};

class ClassStmt : public Stmt {
public:
    Token m_name;
    std::unique_ptr<Variable> m_super_class;
    std::vector<std::unique_ptr<FuncStmt>> m_methods;

    inline ClassStmt(Token& name, std::unique_ptr<Variable> super_class, std::vector<std::unique_ptr<FuncStmt>> methods)
        : m_name(std::move(name))
        , m_super_class(std::move(super_class))
        , m_methods(std::move(methods))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_class_stmt(this);
    }

    virtual ~ClassStmt() = default;
};

class ForEachStmt : public Stmt {
public:
    std::unique_ptr<Stmt> m_var_declaration;
    std::unique_ptr<Expr> m_list_expr;
    std::unique_ptr<Stmt> m_body;

    inline ForEachStmt(std::unique_ptr<Stmt> var_declaration, std::unique_ptr<Expr> list_expr, std::unique_ptr<Stmt> body)
        : m_var_declaration(std::move(var_declaration))
        , m_list_expr(std::move(list_expr))
        , m_body(std::move(body))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor)
    {
        return visitor.visit_for_each_stmt(this);
    }

    virtual ~ForEachStmt() = default;
};

class BreakStmt : public Stmt {
public:
    Token m_break_token;

    inline BreakStmt(Token& break_token)
        : m_break_token(std::move(break_token))
    {
    }

    virtual ~BreakStmt() = default;

    inline virtual std::any accept(IStmtVisitor& visitor)
    {
        return visitor.visit_break_stmt(this);
    }
};

class ExternStmt : public Stmt {
public:
    Token& m_extern_token;
    Token& m_symbol_name;
    std::unique_ptr<FuncStmt> m_june_func;

    inline ExternStmt(Token& extern_token, Token& symbol_name, std::unique_ptr<FuncStmt> june_func)
        : m_extern_token(extern_token)
        , m_symbol_name(symbol_name)
        , m_june_func(std::move(june_func))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor)
    {
        return visitor.visit_extern_stmt(this);
    }

    virtual ~ExternStmt() = default;
};

}
