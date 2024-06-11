#pragma once

#include <functional>

#include "Expr.hpp"

namespace jl {
class Interpreter : public IExprVisitor {
public:
    Interpreter() = default;
    ~Interpreter() = default;

    void interpret(Expr* expr);

private:
    virtual void visit_assign_expr(Assign* expr, void* context) override;
    virtual void visit_binary_expr(Binary* expr, void* context) override;
    virtual void visit_grouping_expr(Grouping* expr, void* context) override;
    virtual void visit_unary_expr(Unary* expr, void* context) override;
    virtual void visit_literal_expr(Literal* expr, void* context) override;
    virtual void* get_context() override;

    void evaluate(Expr* expr, void* context);
    std::string file_name = "Unknown";
    bool is_truthy(Token::Value* value);

    template<typename Op>
    void do_arith_operation(Token::Value& left, Token::Value& right, void *context, Op op);
    void append_strings(Token::Value& left, Token::Value& right, void* context);
    bool is_equal(Token::Value& left, Token::Value& right);
    std::string stringify(Token::Value& value);
};
} // namespace jl
