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
    virtual void visitAssignExpr(Assign& expr) = 0;
    virtual void visitBinaryExpr(Binary& expr) = 0;
    virtual void visitGroupingExpr(Grouping& expr) = 0;
    virtual void visitUnaryExpr(Unary& expr) = 0;
    virtual void visitLiteralExpr(Literal& expr) = 0;
};

class Expr {
public:
    virtual void accept(IExprVisitor& visitor) = 0;
};

class Assign : public Expr {
public:
    inline virtual void accept(IExprVisitor& visitor) override
    {
        visitor.visitAssignExpr(*this);
    }
};

class Binary : public Expr {
public:
    Expr& m_left;
    Token& m_oper;
    Expr& m_right;

    inline Binary(Expr& left, Token& oper, Expr& right)
        : m_left(left)
        , m_right(right)
        , m_oper(oper)
    {
    }

    inline virtual void accept(IExprVisitor& visitor) override
    {
        visitor.visitBinaryExpr(*this);
    }
};

class Grouping : public Expr {
public:
    Expr& m_expr;

    inline Grouping(Expr& expr)
        : m_expr(expr)
    {
    }
    inline virtual void accept(IExprVisitor& visitor) override
    {
        visitor.visitGroupingExpr(*this);
    }
};

class Literal : public Expr {
public:
    Token::Value m_value;
    Token::TokenType m_type;

    inline Literal(Token::Value value, Token::TokenType type)
        : m_value(value)
        , m_type(type)
    {
    }

    inline virtual void accept(IExprVisitor& visitor) override
    {
        visitor.visitLiteralExpr(*this);
    }
};

class Unary : public Expr {
public:
    Expr& m_expr;
    Token& m_oper;
    inline Unary(Token& oper, Expr& expr)
        : m_expr(expr)
        , m_oper(oper)
    {
    }

    inline virtual void accept(IExprVisitor& visitor) override
    {
        visitor.visitUnaryExpr(*this);
    }
};

class ParsetreePrinter : public IExprVisitor {
public:
    inline void visitAssignExpr(Assign& expr) override
    {
    }
    inline void visitBinaryExpr(Binary& expr) override
    {
        parenthesize(expr.m_oper.get_lexeme(), { &expr.m_left, &expr.m_right });
    }
    inline void visitGroupingExpr(Grouping& expr) override
    {
    }
    inline void visitUnaryExpr(Unary& expr) override
    {
        parenthesize(expr.m_oper.get_lexeme(), { &expr.m_expr });
    }
    inline void visitLiteralExpr(Literal& expr) override
    {
        switch (expr.m_type) {
        case Token::INT:
            std::cout << std::get<int>(expr.m_value);
            break;
        case Token::FLOAT:
            std::cout << std::get<double>(expr.m_value);
            break;
        case Token::STRING:
            std::cout << std::get<std::string>(expr.m_value);
            break;

        default:
            break;
        }
    }

    inline void parenthesize(std::string name, std::vector<Expr*> exprs)
    {
        std::cout << "(" << name << " ";
        for (const auto exp : exprs) {
            std::cout << " ";
            exp->accept(*this);
        }
        std::cout << ")";
    }
};

} // namespace jl
