#pragma once

#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Token.hpp"
#include "TypeInfo.hpp"
#include "Value.hpp"
#include "types/Type.hpp"

namespace jl {

class Assign;
class Binary;
class Grouping;
class Unary;
class Literal;
class Variable;
class Logical;
class Call;
class Get;
class Set;
class This;
class Super;
class JList;
class IndexGet;
class IndexSet;
class TypeCast;

class IExprVisitor {
public:
    virtual std::any visit_assign_expr(Assign* expr) = 0;
    virtual std::any visit_binary_expr(Binary* expr) = 0;
    virtual std::any visit_grouping_expr(Grouping* expr) = 0;
    virtual std::any visit_unary_expr(Unary* expr) = 0;
    virtual std::any visit_literal_expr(Literal* expr) = 0;
    virtual std::any visit_variable_expr(Variable* expr) = 0;
    virtual std::any visit_logical_expr(Logical* expr) = 0;
    virtual std::any visit_call_expr(Call* expr) = 0;
    virtual std::any visit_get_expr(Get* expr) = 0;
    virtual std::any visit_set_expr(Set* expr) = 0;
    virtual std::any visit_this_expr(This* expr) = 0;
    virtual std::any visit_super_expr(Super* expr) = 0;
    virtual std::any visit_jlist_expr(JList* expr) = 0;
    virtual std::any visit_index_get_expr(IndexGet* expr) = 0;
    virtual std::any visit_index_set_expr(IndexSet* expr) = 0;
    virtual std::any visit_type_cast_expr(TypeCast* expr) = 0;
};

class Expr {
public:
    const type::Type* m_type = nullptr;
    std::optional<const type::Type*> m_cast_to = std::nullopt;

    virtual std::any accept(IExprVisitor& visitor) = 0;
    virtual ~Expr() = default;
};

class Variable : public Expr {
public:
    Token m_name;

    inline Variable(Token& name)
        : m_name(std::move(name))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_variable_expr(this);
    }

    virtual ~Variable() = default;
};

class Assign : public Expr {
public:
    std::unique_ptr<Expr> m_expr;
    Token m_token;

    inline Assign(std::unique_ptr<Expr> expr, Token& token)
        : m_expr(std::move(expr))
        , m_token(std::move(token))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_assign_expr(this);
    }

    virtual ~Assign() = default;
};

class Binary : public Expr {
public:
    std::unique_ptr<Expr> m_left;
    Token m_oper;
    std::unique_ptr<Expr> m_right;

    inline Binary(std::unique_ptr<Expr> left, Token& oper, std::unique_ptr<Expr> right)
        : m_left(std::move(left))
        , m_right(std::move(right))
        , m_oper(std::move(oper))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_binary_expr(this);
    }

    virtual ~Binary() = default;
};

class Grouping : public Expr {
public:
    std::unique_ptr<Expr> m_expr;

    inline Grouping(std::unique_ptr<Expr> expr)
        : m_expr(std::move(expr))
    {
    }
    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_grouping_expr(this);
    }

    virtual ~Grouping() = default;
};

class Literal : public Expr {
public:
    Value m_value;

    inline Literal(Value& value)
        : m_value(std::move(value))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_literal_expr(this);
    }

    virtual ~Literal() = default;
};

class Unary : public Expr {
public:
    std::unique_ptr<Expr> m_expr;
    Token m_oper;

    inline Unary(Token oper, std::unique_ptr<Expr> expr)
        : m_expr(std::move(expr))
        , m_oper(std::move(oper))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_unary_expr(this);
    }

    virtual ~Unary() = default;
};

class Logical : public Expr {
public:
    std::unique_ptr<Expr> m_left;
    Token m_oper;
    std::unique_ptr<Expr> m_right;

    inline Logical(std::unique_ptr<Expr> left, Token& oper, std::unique_ptr<Expr> right)
        : m_left(std::move(left))
        , m_oper(std::move(oper))
        , m_right(std::move(right))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_logical_expr(this);
    }

    virtual ~Logical() = default;
};

class Call : public Expr {
public:
    std::unique_ptr<Expr> m_callee;
    Token m_paren;
    std::vector<std::unique_ptr<Expr>> m_arguments;

    inline Call(std::unique_ptr<Expr> callee, Token& paren, std::vector<std::unique_ptr<Expr>> arguments)
        : m_callee(std::move(callee))
        , m_paren(std::move(paren))
        , m_arguments(std::move(arguments))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_call_expr(this);
    }

    virtual ~Call() = default;
};

class Get : public Expr {
public:
    Token m_name;
    std::unique_ptr<Expr> m_object;

    inline Get(Token& name, std::unique_ptr<Expr> expr)
        : m_name(std::move(name))
        , m_object(std::move(expr))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_get_expr(this);
    }

    virtual ~Get() = default;
};

class Set : public Expr {
public:
    Token m_name;
    std::unique_ptr<Expr> m_object;
    std::unique_ptr<Expr> m_value;

    inline Set(Token& name, std::unique_ptr<Expr> expr, std::unique_ptr<Expr> value)
        : m_name(std::move(name))
        , m_object(std::move(expr))
        , m_value(std::move(value))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_set_expr(this);
    }

    virtual ~Set() = default;
};

class This : public Expr {
public:
    Token m_keyword;

    inline This(Token& keyword)
        : m_keyword(std::move(keyword))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_this_expr(this);
    }

    virtual ~This() = default;
};

class Super : public Expr {
public:
    Token m_keyword;
    Token m_method;

    inline Super(Token& keyword, Token& method)
        : m_keyword(std::move(keyword))
        , m_method(std::move(method))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_super_expr(this);
    }

    virtual ~Super() = default;
};

class JList : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> m_items;
    Token m_right_brace;
    // During type checking, this field will be set if more elements than declared should be allocated
    std::optional<uint32_t> m_extra_item_count = std::nullopt;

    inline JList(std::vector<std::unique_ptr<Expr>> items, Token right_brace)
        : m_items(std::move(items))
        , m_right_brace(std::move(right_brace))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor)
    {
        return visitor.visit_jlist_expr(this);
    }

    virtual ~JList() = default;
};

class IndexGet : public Expr {
public:
    std::unique_ptr<Expr> m_jlist;
    std::unique_ptr<Expr> m_index_expr;
    Token m_closing_bracket;

    inline IndexGet(std::unique_ptr<Expr> jlist, std::unique_ptr<Expr> index_expr, Token& closing_bracket)
        : m_jlist(std::move(jlist))
        , m_index_expr(std::move(index_expr))
        , m_closing_bracket(std::move(closing_bracket))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor)
    {
        return visitor.visit_index_get_expr(this);
    }

    virtual ~IndexGet() = default;
};

class IndexSet : public Expr {
public:
    std::unique_ptr<Expr> m_jlist;
    std::unique_ptr<Expr> m_index_expr;
    std::unique_ptr<Expr> m_value_expr;
    Token m_closing_bracket;

    inline IndexSet(std::unique_ptr<Expr> jlist, std::unique_ptr<Expr> index_expr, std::unique_ptr<Expr> value_expr, Token& closing_bracket)
        : m_jlist(std::move(jlist))
        , m_index_expr(std::move(index_expr))
        , m_value_expr(std::move(value_expr))
        , m_closing_bracket(std::move(closing_bracket))
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor)
    {
        return visitor.visit_index_set_expr(this);
    }

    virtual ~IndexSet() = default;
};

class TypeCast : public Expr {
public:
    std::unique_ptr<Expr> m_left;
    TypeInfo m_right;

    TypeCast(std::unique_ptr<Expr> left, TypeInfo right)
        : m_left(std::move(left))
        , m_right(right)
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor)
    {
        return visitor.visit_type_cast_expr(this);
    }

    virtual ~TypeCast() = default;
};

}
