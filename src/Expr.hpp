#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Token.hpp"
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

class IExprVisitor {
public:
    virtual void visit_assign_expr(Assign* expr, void* context) = 0;
    virtual void visit_binary_expr(Binary* expr, void* context) = 0;
    virtual void visit_grouping_expr(Grouping* expr, void* context) = 0;
    virtual void visit_unary_expr(Unary* expr, void* context) = 0;
    virtual void visit_literal_expr(Literal* expr, void* context) = 0;
    virtual void visit_variable_expr(Variable* expr, void* context) = 0;
    virtual void visit_logical_expr(Logical* expr, void* context) = 0;
    virtual void visit_call_expr(Call* expr, void* context) = 0;
    virtual void visit_get_expr(Get* expr, void* context) = 0;
    virtual void visit_set_expr(Set* expr, void* context) = 0;
    virtual void visit_this_expr(This* expr, void* context) = 0;
    virtual void visit_super_expr(Super* expr, void* context) = 0;
    virtual void visit_jlist_expr(JList* expr, void* context) = 0;
    virtual void* get_expr_context() = 0;
};

class Expr {
public:
    virtual void accept(IExprVisitor& visitor, void* context) = 0;
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_assign_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_binary_expr(this, context);
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
    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_grouping_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_literal_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_unary_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_variable_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_logical_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_call_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_get_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_set_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_this_expr(this, context);
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

    inline virtual void accept(IExprVisitor& visitor, void* context) override
    {
        visitor.visit_super_expr(this, context);
    }

    virtual ~Super() = default;
};

class JList: public Expr {
public:
    inline JList(std::vector<Expr*>& items)
        : m_items(items) {}
    inline JList(std::vector<Expr*>&& items)
        : m_items(std::move(items)) {}

    inline virtual void accept(IExprVisitor& visitor, void* context)
    {
        visitor.visit_jlist_expr(this, context);
    }

    inline std::vector<Expr*>& get()
    {
        return m_items;
    }

    virtual ~JList() = default;
private:
    std::vector<Expr*> m_items;
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

class ParsetreePrinter : public IExprVisitor {
public:
    std::string context;

    inline void visit_assign_expr(Assign* expr, void* context) override
    {
    }
    inline void visit_binary_expr(Binary* expr, void* context) override
    {
        parenthesize(expr->m_oper->get_lexeme(), { expr->m_left, expr->m_right });
    }
    inline void visit_grouping_expr(Grouping* expr, void* context) override
    {
        parenthesize("group", { expr->m_expr });
    }
    inline void visit_unary_expr(Unary* expr, void* context) override
    {
        parenthesize(expr->m_oper->get_lexeme(), { expr->m_expr });
    }
    inline void visit_variable_expr(Variable* expr, void* context) override
    {
        parenthesize(expr->m_name.get_lexeme(), {});
    }
    inline void visit_logical_expr(Logical* expr, void* context) override
    {
    }
    inline void visit_call_expr(Call* expr, void* context) override
    {
    }
    inline void visit_get_expr(Get* expr, void* context) override
    {
    }
    inline void visit_set_expr(Set* expr, void* context) override
    {
    }
    inline void visit_this_expr(This* expr, void* context) override
    {
    }
    inline void visit_super_expr(Super* expr, void* context) override
    {
    }
    inline void visit_literal_expr(Literal* expr, void* context) override
    {
        std::string* cntx = reinterpret_cast<std::string*>(context);
        if (is_int(*expr->m_value)) {
            cntx->append(std::to_string(std::get<int>(*expr->m_value)));
        } else if (is_float(*expr->m_value)) {
            cntx->append(std::to_string(std::get<double>(*(expr->m_value))));
        } else if (is_string(*expr->m_value)) {
            cntx->append(std::get<std::string>(*(expr->m_value)));
        } else {
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

    inline virtual void* get_expr_context() override
    {
        return &context;
    }
};

} // namespace jl
