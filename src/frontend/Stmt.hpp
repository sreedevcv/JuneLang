#pragma once

#include "Expr.hpp"
#include "Ref.hpp"
#include "Token.hpp"

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
};

class Stmt : public Ref {
public:
    virtual std::any accept(IStmtVisitor& visitor) = 0;
    virtual ~Stmt() = default;
};

class ExprStmt : public Stmt {
public:
    Expr* m_expr;

    inline ExprStmt(Expr* expr)
        : m_expr(expr)
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_expr_stmt(this);
    }

    virtual ~ExprStmt()
    {
        // delete m_expr;
    }
};

class PrintStmt : public Stmt {
public:
    Expr* m_expr;

    inline PrintStmt(Expr* expr)
        : m_expr(expr)
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_print_stmt(this);
    }

    virtual ~PrintStmt()
    {
        // delete m_expr;
    }
};

class VarStmt : public Stmt {
public:
    Token& m_name;
    Expr* m_initializer;
    Token* m_data_type;

    inline VarStmt(Token& name, Expr* initializer, Token* data_type)
        : m_name(name)
        , m_initializer(initializer)
        , m_data_type(data_type)
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_var_stmt(this);
    }

    virtual ~VarStmt()
    {
        // delete m_initializer;
    }
};

class BlockStmt : public Stmt {
public:
    std::vector<Stmt*> m_statements;

    inline BlockStmt(std::vector<Stmt*>&& statements)
        : m_statements(std::move(statements))
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_block_stmt(this);
    }

    virtual ~BlockStmt()
    {
        // for (auto stmt : m_statements) {
        //     delete stmt;
        // }
    }
};

class EmptyStmt : public Stmt {
public:
    EmptyStmt() = default;
    virtual ~EmptyStmt() = default;

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_empty_stmt(this);
    }
};

class IfStmt : public Stmt {
public:
    Expr* m_condition;
    Stmt* m_then_stmt;
    Stmt* m_else_stmt;

    inline IfStmt(Expr* condition, Stmt* then_stmt, Stmt* else_stmt)
        : m_condition(condition)
        , m_then_stmt(then_stmt)
        , m_else_stmt(else_stmt)
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_if_stmt(this);
    }

    virtual ~IfStmt()
    {
        // delete m_condition;
        // delete m_then_stmt;
        // delete m_else_stmt;
    }
};

class WhileStmt : public Stmt {
public:
    Expr* m_condition;
    Stmt* m_body;

    inline WhileStmt(Expr* condition, Stmt* body)
        : m_condition(condition)
        , m_body(body)
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_while_stmt(this);
    }

    virtual ~WhileStmt()
    {
        // delete m_condition;
        // delete m_body;
    }
};

class FuncStmt : public Stmt {
public:
    Token& m_name;
    std::vector<Token*> m_params; // Should i delete these??
    std::vector<Stmt*> m_body;

    inline FuncStmt(Token& name, std::vector<Token*>& params, std::vector<Stmt*>& body)
        : m_name(name)
        , m_params(params)
        , m_body(body)
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_func_stmt(this);
    }

    virtual ~FuncStmt()
    {
        // for (auto stmt: m_body)
        // {
        //     delete stmt;
        // }
    }
};

class ReturnStmt : public Stmt {
public:
    Token& m_keyword;
    Expr* m_expr;

    inline ReturnStmt(Token& keyword, Expr* expr)
        : m_keyword(keyword)
        , m_expr(expr)
    {
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_return_stmt(this);
    }

    virtual ~ReturnStmt()
    {
        //     delete m_expr;
    }
};

class ClassStmt : public Stmt {
public:
    Token& m_name;
    Variable* m_super_class;
    std::vector<FuncStmt*> m_methods;

    inline ClassStmt(Token& name, Variable* super_class, std::vector<FuncStmt*>& methods)
        : m_name(name)
        , m_super_class(super_class)
        , m_methods(methods)
    {
    }

    virtual ~ClassStmt()
    {
        // delete m_super_class;
        // for (auto method: m_methods)
        // {
        //     delete method;
        // }
    }

    inline virtual std::any accept(IStmtVisitor& visitor) override
    {
        return visitor.visit_class_stmt(this);
    }
};

class ForEachStmt : public Stmt {
public:
    VarStmt* m_var_declaration;
    Expr* m_list_expr;
    Stmt* m_body;

    inline ForEachStmt(VarStmt* var_declaration, Expr* list_expr, Stmt* body)
        : m_var_declaration(var_declaration)
        , m_list_expr(list_expr)
        , m_body(body)
    {
    }
    virtual ~ForEachStmt() = default;

    inline virtual std::any accept(IStmtVisitor& visitor)
    {
        return visitor.visit_for_each_stmt(this);
    }
};

class BreakStmt : public Stmt {
public:
    Token& m_break_token;

    inline BreakStmt(Token& break_token)
        : m_break_token(break_token)
    {
    }

    virtual ~BreakStmt() = default;

    inline virtual std::any accept(IStmtVisitor& visitor)
    {
        return visitor.visit_break_stmt(this);
    }
};

}