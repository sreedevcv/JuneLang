#pragma once

#include "Expr.hpp"

namespace jl {

class PrintStmt;
class ExprStmt;
class VarStmt;
class BlockStmt;
class EmptyStmt;

class IStmtVisitor {
public:
    virtual void visit_print_stmt(PrintStmt* stmt, void* context) = 0;
    virtual void visit_expr_stmt(ExprStmt* stmt, void* context) = 0;
    virtual void visit_var_stmt(VarStmt* stmt, void* context) = 0;
    virtual void visit_block_stmt(BlockStmt* stmt, void* context) = 0;
    virtual void visit_empty_stmt(EmptyStmt* stmt, void* context) = 0;
    virtual void* get_stmt_context() = 0;
};

class Stmt {
public:
    virtual void accept(IStmtVisitor& visitor, void* context) = 0;
};

class ExprStmt : public Stmt {
public:
    Expr* m_expr;

    inline ExprStmt(Expr* expr)
        : m_expr(expr)
    {
    }

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_expr_stmt(this, context);
    }

    virtual ~ExprStmt() = default;
};

class PrintStmt : public Stmt {
public:
    Expr* m_expr;

    inline PrintStmt(Expr* expr)
        : m_expr(expr)
    {
    }

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_print_stmt(this, context);
    }

    virtual ~PrintStmt() = default;
};

class VarStmt : public Stmt {
public:
    Token& m_name;
    Expr* m_initializer;

    inline VarStmt(Token& name, Expr* initializer)
        : m_name(name)
        , m_initializer(initializer)
    {
    }

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_var_stmt(this, context);
    }

    virtual ~VarStmt() = default;
};

class BlockStmt : public Stmt {
public:
    std::vector<Stmt*> m_statements;

    inline BlockStmt(std::vector<Stmt*>&& statements)
        : m_statements(std::move(statements))
    {
    }

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_block_stmt(this, context);
    }

    virtual ~BlockStmt() = default;
};

class EmptyStmt : public Stmt {
public:
    EmptyStmt() = default;
    virtual ~EmptyStmt() = default;

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_empty_stmt(this, context);
    }
};

}