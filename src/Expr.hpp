#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Token.hpp"

namespace jl {
class Assign;
class Binary;
class Grouping;
class Unary;
class Literal;

class IExprVisitor {
public:
    virtual void visitAssignExpr(Assign* expr, void* context) = 0;
    virtual void visitBinaryExpr(Binary* expr, void* context) = 0;
    virtual void visitGroupingExpr(Grouping* expr, void* context) = 0;
    virtual void visitUnaryExpr(Unary* expr, void* context) = 0;
    virtual void visitLiteralExpr(Literal* expr, void* context) = 0;

    virtual void* get_context() = 0;
};

class Expr {
public:
    virtual void accept(IExprVisitor& visitor, void* context) = 0;
};

class Assign : public Expr {
public:
    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visitAssignExpr(this, context);
    }
};

class Binary : public Expr {
public:
    Expr* m_left;
    Token* m_oper;
    Expr* m_right;

    inline Binary(Expr* left, Token* oper, Expr* right)
        : m_left(left)
        , m_right(right)
        , m_oper(oper)
    {
    }

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visitBinaryExpr(this, context);
    }
};

class Grouping : public Expr {
public:
    Expr* m_expr;

    inline Grouping(Expr* expr)
        : m_expr(expr)
    {
    }
    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visitGroupingExpr(this, context);
    }
};

class Literal : public Expr {
public:
    Token::Value* m_value;
    Token::TokenType m_type;

    inline Literal(Token::Value* value, Token::TokenType type)
        : m_value(value)
        , m_type(type)
    {
    }

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visitLiteralExpr(this, context);
    }
};

class Unary : public Expr {
public:
    Expr* m_expr;
    Token* m_oper;

    inline Unary(Token* oper, Expr* expr)
        : m_expr(expr)
        , m_oper(oper)
    {
    }

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visitUnaryExpr(this, context);
    }
};


class ParsetreePrinter : public IExprVisitor {
public:
    std::string context;

    inline void visitAssignExpr(Assign* expr, void* context) override
    {
    }
    inline void visitBinaryExpr(Binary* expr, void* context) override
    {
        parenthesize(expr->m_oper->get_lexeme(), { expr->m_left, expr->m_right });
    }
    inline void visitGroupingExpr(Grouping* expr, void* context) override
    {
    }
    inline void visitUnaryExpr(Unary* expr, void* context) override
    {
        parenthesize(expr->m_oper->get_lexeme(), { expr->m_expr });
    }
    inline void visitLiteralExpr(Literal* expr, void* context) override
    {
        std::string* cntx = reinterpret_cast<std::string*>(context);

        switch (expr->m_type) {
        case Token::INT:
            cntx->append(std::to_string(std::get<int>(*(expr->m_value))));
            break;
        case Token::FLOAT:
            cntx->append(std::to_string(std::get<double>(*(expr->m_value))));
            break;
        case Token::STRING:
            cntx->append(std::get<std::string>(*(expr->m_value)));
            break;

        default:
            break;
        }
    }

    inline void parenthesize(std::string name, std::vector<Expr*> exprs)
    {
        context.append("(");
        context.append(name);
        for (const auto exp : exprs) {
            context.append(" ");
            exp->accept(*this, &context);
        }
        context.append(")");
    }

    inline virtual void* get_context()
    {
        return &context;
    }
};

} // namespace jl
