#pragma once

#include <any>
#include <vector>

#include "Token.hpp"
#include "TypeInfo.hpp"
#include "Value.hpp"

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

class Expr : public Ref {
public:
    virtual std::any accept(IExprVisitor& visitor) = 0;
    virtual ~Expr() = default;
};

class Assign : public Expr {
public:
    Expr* m_expr;
    Token& m_token;

    inline Assign(Expr* expr, Token& token)
        : m_expr(expr)
        , m_token(token)
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_assign_expr(this);
    }

    virtual ~Assign()
    {
        // delete m_expr;
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

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_binary_expr(this);
    }

    virtual ~Binary()
    {
        // delete m_left;
        // delete m_right;
    }
};

class Grouping : public Expr {
public:
    Expr* m_expr;

    inline Grouping(Expr* expr)
        : m_expr(expr)
    {
    }
    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_grouping_expr(this);
    }

    virtual ~Grouping()
    {
        // delete m_expr;
    }
};

class Literal : public Expr {
public:
    Value* m_value;

    inline Literal(Value* value)
        : m_value(value)
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_literal_expr(this);
    }

    virtual ~Literal()
    {
        // delete m_value;
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

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_unary_expr(this);
    }

    virtual ~Unary()
    {
        // delete m_expr;
        // delete m_oper;
    }
};

class Variable : public Expr {
public:
    Token& m_name;

    inline Variable(Token& name)
        : m_name(name)
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_variable_expr(this);
    }

    virtual ~Variable() = default;
};

class Logical : public Expr {
public:
    Expr* m_left;
    Token& m_oper;
    Expr* m_right;

    inline Logical(Expr* left, Token& oper, Expr* right)
        : m_left(left)
        , m_oper(oper)
        , m_right(right)
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_logical_expr(this);
    }

    virtual ~Logical()
    {
        // delete m_left;
        // delete m_right;
    }
};

class Call : public Expr {
public:
    Expr* m_callee;
    Token& m_paren;
    std::vector<Expr*> m_arguments;

    inline Call(Expr* callee, Token& paren, std::vector<Expr*>& arguments)
        : m_callee(callee)
        , m_paren(paren)
        , m_arguments(arguments)
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_call_expr(this);
    }

    virtual ~Call()
    {
        // delete m_callee;
        // for (auto exp: m_arguments)
        // {
        //     delete exp;
        // }
    }
};

class Get : public Expr {
public:
    Token& m_name;
    Expr* m_object;

    inline Get(Token& name, Expr* expr)
        : m_name(name)
        , m_object(expr)
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_get_expr(this);
    }

    virtual ~Get()
    {
        // delete m_object;
    }
};

class Set : public Expr {
public:
    Token& m_name;
    Expr* m_object;
    Expr* m_value;

    inline Set(Token& name, Expr* expr, Expr* value)
        : m_name(name)
        , m_object(expr)
        , m_value(value)
    {
    }

    inline virtual std::any accept(IExprVisitor& visitor) override
    {
        return visitor.visit_set_expr(this);
    }

    virtual ~Set()
    {
        // delete m_object;
        // delete m_value;
    }
};

class This : public Expr {
public:
    Token& m_keyword;

    inline This(Token& keyword)
        : m_keyword(keyword)
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
    Token& m_keyword;
    Token& m_method;

    inline Super(Token& keyword, Token& method)
        : m_keyword(keyword)
        , m_method(method)
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
    std::vector<Expr*> m_items;

    inline JList(std::vector<Expr*>& items)
        : m_items(items)
    {
    }
    inline JList(std::vector<Expr*>&& items)
        : m_items(std::move(items))
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
    Expr* m_jlist;
    Expr* m_index_expr;
    Token& m_closing_bracket;

    inline IndexGet(Expr* jlist, Expr* index_expr, Token& closing_bracket)
        : m_jlist(jlist)
        , m_index_expr(index_expr)
        , m_closing_bracket(closing_bracket)
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
    Expr* m_jlist;
    Expr* m_index_expr;
    Expr* m_value_expr;
    Token& m_closing_bracket;

    inline IndexSet(Expr* jlist, Expr* index_expr, Expr* value_expr, Token& closing_bracket)
        : m_jlist(jlist)
        , m_index_expr(index_expr)
        , m_value_expr(value_expr)
        , m_closing_bracket(closing_bracket)
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
    Expr* m_left;
    TypeInfo m_right;

    TypeCast(Expr* left, TypeInfo right)
        : m_left(left)
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