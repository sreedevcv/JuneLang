#include "LLVMIRGen.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include "Value.hpp"
#include "llvm_backend/JuneModule.hpp"
#include <any>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>

jl::LLVMIRGen::LLVMIRGen(const std::string file_name)
    : m_module(file_name)
{
}

jl::JuneModule& jl::LLVMIRGen::emit(const std::vector<std::unique_ptr<Stmt>>& stmts)
{
    for (const auto& stmt : stmts) {
        emit(stmt);
    }

    return m_module;
}

llvm::Value* jl::LLVMIRGen::emit(const std::unique_ptr<Expr>& expr)
{
    auto any = expr->accept(*this);
    return std::any_cast<llvm::Value*>(any);
}

void jl::LLVMIRGen::emit(const std::unique_ptr<Stmt>& stmt)
{
    stmt->accept(*this);
}

std::any jl::LLVMIRGen::visit_literal_expr(Literal* expr)
{
    llvm::Value* value = nullptr;

    switch (get_type(expr->m_value)) {
    case Type::INT:
        value = llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(m_module.ctx()),
            std::get<int>(expr->m_value),
            true);
        break;
    case Type::FLOAT:
        value = llvm::ConstantFP::get(
            llvm::Type::getDoubleTy(m_module.ctx()),
            std::get<double>(expr->m_value));
        break;
    case Type::BOOL:
    case Type::CHAR:
    case Type::STR:
    default:
        unimplemented("literal type");
        break;
    }

    return value;
}

std::any jl::LLVMIRGen::visit_assign_expr(Assign* expr)
{
    auto value = emit(expr->m_expr);
    auto [var_ptr, _] = m_module.function().read_local_var_definiton(expr->m_token.get_lexeme());
    m_module.builder().CreateStore(value, var_ptr);
    return value;
}

std::any jl::LLVMIRGen::visit_binary_expr(Binary* expr)
{
    auto left = emit(expr->m_left);
    auto right = emit(expr->m_right);

    switch (expr->m_oper.get_tokentype()) {
    case Token::PLUS:
        return m_module.builder().CreateAdd(left, right);
    case Token::MINUS:
        return m_module.builder().CreateSub(left, right);
    case Token::STAR:
        return m_module.builder().CreateMul(left, right);
    case Token::SLASH:
        return m_module.builder().CreateSDiv(left, right);
    case Token::GREATER:
        return m_module.builder().CreateICmpSGT(left, right);
    case Token::LESS:
        return m_module.builder().CreateICmpSLT(left, right);
    case Token::GREATER_EQUAL:
        return m_module.builder().CreateICmpSGT(left, right);
    case Token::LESS_EQUAL:
        return m_module.builder().CreateICmpSLT(left, right);
    case Token::EQUAL_EQUAL:
        return m_module.builder().CreateICmpEQ(left, right);
    case Token::BANG_EQUAL:
        return m_module.builder().CreateICmpNE(left, right);
    case Token::PERCENT:
    case Token::BIT_AND:
    case Token::BIT_OR:
    case Token::BIT_XOR:
    default:
        unimplemented("Unsupported binary operation");
    }
    return {};
}

std::any jl::LLVMIRGen::visit_unary_expr(Unary* expr)
{
    auto value = emit(expr->m_expr);
    switch (expr->m_oper.get_tokentype()) {
    case Token::MINUS:
        return m_module.builder().CreateNeg(value);
    case Token::BANG:
        return m_module.builder().CreateNot(value);
    case Token::BIT_NOT:
    default:
        unimplemented("Unsupported unary operation");
    }
    return {};
}

std::any jl::LLVMIRGen::visit_logical_expr(Logical* expr)
{
    auto left = emit(expr->m_left);
    auto right = emit(expr->m_right);

    if (expr->m_oper.get_tokentype() == Token::OR) {
        return m_module.builder().CreateLogicalOr(left, right);
    } else {
        return m_module.builder().CreateLogicalAnd(left, right);
    }
}

std::any jl::LLVMIRGen::visit_variable_expr(Variable* expr)
{
    auto [ptr, type] = m_module.function().read_local_var_definiton(expr->m_name.get_lexeme());
    return static_cast<llvm::Value*>(m_module.builder().CreateLoad(type, ptr));
}

std::any jl::LLVMIRGen::visit_grouping_expr(Grouping* expr)
{
    return emit(expr->m_expr);
}

std::any jl::LLVMIRGen::visit_call_expr(Call* expr)
{
    unimplemented("visit_call_expr");
    return {};
}
std::any jl::LLVMIRGen::visit_get_expr(Get* expr)
{
    unimplemented("visit_get_expr");
    return {};
}
std::any jl::LLVMIRGen::visit_set_expr(Set* expr)
{
    unimplemented("visit_set_expr");
    return {};
}
std::any jl::LLVMIRGen::visit_this_expr(This* expr)
{
    unimplemented("visit_this_expr");
    return {};
}
std::any jl::LLVMIRGen::visit_super_expr(Super* expr)
{
    unimplemented("visit_super_expr");
    return {};
}
std::any jl::LLVMIRGen::visit_jlist_expr(JList* expr)
{
    unimplemented("visit_jlist_expr");
    return {};
}
std::any jl::LLVMIRGen::visit_index_get_expr(IndexGet* expr)
{
    unimplemented("visit_index_get_expr");
    return {};
}
std::any jl::LLVMIRGen::visit_index_set_expr(IndexSet* expr)
{
    unimplemented("visit_index_set_expr");
    return {};
}
std::any jl::LLVMIRGen::visit_type_cast_expr(TypeCast* expr)
{
    unimplemented("visit_type_cast_expr");
    return {};
}

// -------------------------------------STMTS-------------------------------------------------

std::any jl::LLVMIRGen::visit_func_stmt(FuncStmt* stmt)
{
    llvm::SmallVector<llvm::Type*, 8> llvm_types;
    // Collect all the input parameters types
    for (const auto& param_type : stmt->m_data_types) {
        auto llvm_type = m_module.map_to_llvm_type(param_type);
        llvm_types.push_back(*llvm_type);
    }

    // Find the llvm return type
    auto return_type = stmt->m_return_type
        ? m_module.map_to_llvm_type(*stmt->m_return_type).value()
        : llvm::Type::getVoidTy(m_module.ctx());

    // Create function type
    auto function_type = llvm::FunctionType::get(return_type, llvm_types, false);
    // Create fucnction
    auto function = llvm::Function::Create(
        function_type,
        llvm::GlobalValue::ExternalLinkage,
        stmt->m_name.get_lexeme(),
        m_module.module());

    auto entry_block = llvm::BasicBlock::Create(m_module.ctx(), "entry", function);
    m_module.builder().SetInsertPoint(entry_block);
    m_module.set_function(function);

    for (auto [idx, arg] : llvm::enumerate(function->args())) {
        const auto param = stmt->m_params[idx];
        // TODO::Add attributes
        arg.setName(param->get_lexeme());
        // Store it in stack
        auto alloca_arg = m_module.builder().CreateAlloca(arg.getType(), nullptr, param->get_lexeme());
        m_module.function().add_local_var_definition(param->get_lexeme(), alloca_arg, arg.getType());
    }

    emit(stmt->m_body);

    return {};
}

std::any jl::LLVMIRGen::visit_var_stmt(VarStmt* stmt)
{
    if (!stmt->m_initializer) {
        llvm::WithColor::error(llvm::errs()) << "Untyped variable initializations not supported now";
        unimplemented("Untyped variable initializations");
    }

    // Evaluate the initializer
    auto val = emit(stmt->m_initializer.value());
    // Allocate the var on the stack
    auto var_alloc = m_module.builder().CreateAlloca(val->getType(), nullptr, stmt->m_name.get_lexeme());
    // Store the initialized value in the ptr
    m_module.builder().CreateStore(val, var_alloc);
    // Store the name and ptr in the scope
    m_module.function().add_local_var_definition(stmt->m_name.get_lexeme(), var_alloc, val->getType());

    return {};
}

std::any jl::LLVMIRGen::visit_return_stmt(ReturnStmt* stmt)
{
    if (stmt->m_expr) {
        auto ret_value = emit(stmt->m_expr.value());
        m_module.builder().CreateRet(ret_value);
    } else {
        m_module.builder().CreateRetVoid();
    }
    return {};
}

std::any jl::LLVMIRGen::visit_block_stmt(BlockStmt* stmt)
{
    m_module.function().push_scope();
    emit(stmt->m_statements);
    m_module.function().pop_scope();
    return {};
}

std::any jl::LLVMIRGen::visit_if_stmt(IfStmt* stmt)
{
    // Evaluate the condition
    auto condition = emit(stmt->m_condition);
    // Create the if block
    auto if_block = llvm::BasicBlock::Create(m_module.ctx(), "cond.true", m_module.llvm_function());
    // Create the else/after-if block
    auto else_block = llvm::BasicBlock::Create(m_module.ctx(), "cond.false", m_module.llvm_function());
    // Create the branch instruction
    m_module.builder().CreateCondBr(condition, if_block, else_block);
    // Evalue the if block
    m_module.builder().SetInsertPoint(if_block);
    emit(stmt->m_then_stmt);
    // m_module.builder().CreateBr(else_block);

    if (stmt->m_else_stmt) {
        // Evaluate the else block
        m_module.builder().SetInsertPoint(else_block);
        emit(stmt->m_else_stmt.value());

        // // Jump to a new block after the if block is done
        auto continuation_block = llvm::BasicBlock::Create(m_module.ctx(), "after-if", m_module.llvm_function());
        m_module.builder().SetInsertPoint(if_block);
        m_module.builder().CreateBr(continuation_block);

        // // Set the bolc to continue with
        else_block = continuation_block;
    }

    m_module.builder().SetInsertPoint(else_block);

    return {};
}

std::any jl::LLVMIRGen::visit_print_stmt(PrintStmt* stmt)
{
    unimplemented("visit_print_stmt");
    return {};
}
std::any jl::LLVMIRGen::visit_expr_stmt(ExprStmt* stmt)
{
    emit(stmt->m_expr);
    return {};
}
std::any jl::LLVMIRGen::visit_empty_stmt(EmptyStmt* stmt)
{
    return {};
}
std::any jl::LLVMIRGen::visit_while_stmt(WhileStmt* stmt)
{
    unimplemented("visit_while_stmt");
    return {};
}

std::any jl::LLVMIRGen::visit_class_stmt(ClassStmt* stmt)
{
    unimplemented("visit_class_stmt");
    return {};
}
std::any jl::LLVMIRGen::visit_for_each_stmt(ForEachStmt* stmt)
{
    unimplemented("visit_for_each_stmt");
    return {};
}
std::any jl::LLVMIRGen::visit_break_stmt(BreakStmt* stmt)
{
    unimplemented("visit_break_stmt");
    return {};
}
std::any jl::LLVMIRGen::visit_extern_stmt(ExternStmt* stmt)
{
    unimplemented("visit_extern_stmt");
    return {};
}
