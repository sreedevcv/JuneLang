#pragma once

#include "Expr.hpp"

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

class IStmtVisitor {
public:
    virtual void visit_print_stmt(PrintStmt* stmt, void* context) = 0;
    virtual void visit_expr_stmt(ExprStmt* stmt, void* context) = 0;
    virtual void visit_var_stmt(VarStmt* stmt, void* context) = 0;
    virtual void visit_block_stmt(BlockStmt* stmt, void* context) = 0;
    virtual void visit_empty_stmt(EmptyStmt* stmt, void* context) = 0;
    virtual void visit_if_stmt(IfStmt* stmt, void* context) = 0;
    virtual void visit_while_stmt(WhileStmt* stmt, void* context) = 0;
    virtual void visit_func_stmt(FuncStmt* stmt, void* context) = 0;
    virtual void visit_return_stmt(ReturnStmt* stmt, void* context) = 0;
    virtual void visit_class_stmt(ClassStmt* stmt, void* context) = 0;
    virtual void* get_stmt_context() = 0;
};

class Stmt {
public:
    virtual void accept(IStmtVisitor& visitor, void* context) = 0;
    virtual ~Stmt() = default;
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

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_print_stmt(this, context);
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

    inline VarStmt(Token& name, Expr* initializer)
        : m_name(name)
        , m_initializer(initializer)
    {
    }

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_var_stmt(this, context);
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

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_block_stmt(this, context);
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

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_empty_stmt(this, context);
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

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_if_stmt(this, context);
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

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_while_stmt(this, context);
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
    std::vector<Token*> m_params;   // Should i delete these??
    std::vector<Stmt*> m_body;

    inline FuncStmt(Token& name, std::vector<Token*>& params, std::vector<Stmt*>& body)
        : m_name(name)
        , m_params(params)
        , m_body(body)
    {
    }

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_func_stmt(this, context);
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

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_return_stmt(this, context);
    }

    // virtual ~ReturnStmt()
    // {
    //     delete m_expr;
    // }
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

    inline virtual void accept(IStmtVisitor& visitor, void* context) override
    {
        visitor.visit_class_stmt(this, context);
    }
};

}