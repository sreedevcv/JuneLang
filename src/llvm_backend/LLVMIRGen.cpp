#include "LLVMIRGen.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include "Value.hpp"
#include "llvm_backend/JuneModule.hpp"
#include "types/Type.hpp"
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
    auto curr_block = m_module.builder().GetInsertBlock();
    // Emit instructions only if the current BB has not yet terminated
    if (!curr_block || curr_block->getTerminator() == nullptr) {
        stmt->accept(*this);
    }
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
        value = llvm::ConstantInt::getBool(
            m_module.ctx(),
            std::get<bool>(expr->m_value));
        break;
    case Type::CHAR:
        value = llvm::ConstantInt::get(
            llvm::Type::getInt8Ty(m_module.ctx()),
            std::get<char>(expr->m_value));
        break;
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
    auto [var_ptr, _] = m_module.function().read_local_var_def(expr->m_token.get_lexeme());
    m_module.builder().CreateStore(value, var_ptr);
    return value;
}

std::any jl::LLVMIRGen::visit_binary_expr(Binary* expr)
{
    auto left = emit(expr->m_left);
    auto right = emit(expr->m_right);
    const auto is_float = static_cast<type::Builtin*>(expr->m_left->m_type.get())->m_primitive == type::Builtin::FLOAT;

    // TODO::Support for floating point numbers
    switch (expr->m_oper.get_tokentype()) {
    case Token::PLUS:
        return m_module.builder().CreateAdd(left, right);
    case Token::MINUS:
        return m_module.builder().CreateSub(left, right);
    case Token::STAR:
        return m_module.builder().CreateMul(left, right);
    case Token::SLASH:
        if (is_float)
            return m_module.builder().CreateFDiv(left, right);
        return m_module.builder().CreateSDiv(left, right);
    case Token::GREATER:
        if (is_float)
            return m_module.builder().CreateFCmpOGT(left, right);
        return m_module.builder().CreateICmpSGT(left, right);
    case Token::LESS:
        if (is_float)
            return m_module.builder().CreateFCmpOLT(left, right);
        return m_module.builder().CreateICmpSLT(left, right);
    case Token::GREATER_EQUAL:
        if (is_float)
            return m_module.builder().CreateFCmpOGE(left, right);
        return m_module.builder().CreateICmpSGT(left, right);
    case Token::LESS_EQUAL:
        if (is_float)
            return m_module.builder().CreateFCmpOLE(left, right);
        return m_module.builder().CreateICmpSLT(left, right);
    case Token::EQUAL_EQUAL:
        if (is_float)
            return m_module.builder().CreateFCmpOEQ(left, right);
        return m_module.builder().CreateICmpEQ(left, right);
    case Token::BANG_EQUAL:
        if (is_float)
            return m_module.builder().CreateFCmpONE(left, right);
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
        if (static_cast<type::Builtin*>(expr->m_expr->m_type.get())->m_primitive == type::Builtin::FLOAT)
            return m_module.builder().CreateFNeg(value);
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
    // First check if its a function
    auto func = m_module.get_function(expr->m_name.get_lexeme());
    if (func) {
        return static_cast<llvm::Value*>(func.value());
    }

    auto [ptr, type] = m_module.function().read_local_var_def(expr->m_name.get_lexeme());
    return static_cast<llvm::Value*>(m_module.builder().CreateLoad(type, ptr));
}

std::any jl::LLVMIRGen::visit_grouping_expr(Grouping* expr)
{
    return emit(expr->m_expr);
}

std::any jl::LLVMIRGen::visit_call_expr(Call* expr)
{
    llvm::SmallVector<llvm::Value*, 5> arg_values;

    for (const auto& arg : expr->m_arguments) {
        arg_values.push_back(emit(arg));
    }

    auto name_val = emit(expr->m_callee);

    return static_cast<llvm::Value*>(m_module.builder().CreateCall((static_cast<llvm::Function*>(name_val)), arg_values));
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

    // Store the function name as variable so that it can be looked up during
    m_module.store_function(stmt->m_name.get_lexeme(), function);
    m_module.set_current_function(function);

    auto entry_block = llvm::BasicBlock::Create(m_module.ctx(), "entry", function);
    m_module.builder().SetInsertPoint(entry_block);

    for (auto [idx, arg] : llvm::enumerate(function->args())) {
        const auto param = stmt->m_params[idx];
        // TODO::Add attributes
        arg.setName(param->get_lexeme());
        // Store it in stack
        auto alloca_arg = m_module.allocate_in_entry_block(param->get_lexeme(), arg.getType());
        m_module.function().add_local_var_def(param->get_lexeme(), alloca_arg, arg.getType());
        m_module.builder().CreateStore(&arg, alloca_arg);
    }

    auto rest_block = llvm::BasicBlock::Create(m_module.ctx(), "rest", function);
    m_module.builder().CreateBr(rest_block);
    m_module.builder().SetInsertPoint(rest_block);

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
    auto var_alloc = m_module.allocate_in_entry_block(stmt->m_name.get_lexeme(), val->getType());
    // Store the initialized value in the ptr
    m_module.builder().CreateStore(val, var_alloc);
    // Store the name and ptr in the scope
    m_module.function().add_local_var_def(stmt->m_name.get_lexeme(), var_alloc, val->getType());

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

    if (stmt->m_else_stmt) {
        auto after_block = llvm::BasicBlock::Create(m_module.ctx(), "cond.after", m_module.llvm_function());

        // Add the br to cond.after in the current block(continued from the original if-then instrs)
        if (!m_module.builder().GetInsertBlock()->getTerminator())
            m_module.builder().CreateBr(after_block);

        // Emit the instrs for the else block
        m_module.builder().SetInsertPoint(else_block);
        emit(stmt->m_else_stmt.value());

        // Add the br to cond.after in the current block(continued from the original else instrs)
        if (!m_module.builder().GetInsertBlock()->getTerminator())
            m_module.builder().CreateBr(after_block);

        // New instructions will be added to the cond.after block
        m_module.builder().SetInsertPoint(after_block);
    } else {
        // Add the br to cond.else in the current block(continued from the original if-then instrs)
        if (!m_module.builder().GetInsertBlock()->getTerminator())
            m_module.builder().CreateBr(else_block);

        // New instructions will be added to the cond.else block
        m_module.builder().SetInsertPoint(else_block);
    }

    return {};
}

std::any jl::LLVMIRGen::visit_while_stmt(WhileStmt* stmt)
{
    auto condition_block = llvm::BasicBlock::Create(m_module.ctx(), "while.cond", m_module.llvm_function());
    m_module.builder().CreateBr(condition_block);

    auto while_block = llvm::BasicBlock::Create(m_module.ctx(), "while.body", m_module.llvm_function());
    auto after_block = llvm::BasicBlock::Create(m_module.ctx(), "while.after", m_module.llvm_function());
    // The loop guard
    m_module.builder().SetInsertPoint(condition_block);
    auto condition = emit(stmt->m_condition);
    // Generate the branch instruction
    m_module.builder().CreateCondBr(condition, while_block, after_block);
    // Emit the while block instructions
    m_module.builder().SetInsertPoint(while_block);
    emit(stmt->m_body);
    // Loop back to the while.cond block
    if (!m_module.builder().GetInsertBlock()->getTerminator())
        m_module.builder().CreateBr(condition_block);

    // New instructions will go to the while.after block
    m_module.builder().SetInsertPoint(after_block);

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

std::any jl::LLVMIRGen::visit_print_stmt(PrintStmt* stmt)
{
    unimplemented("visit_print_stmt");
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

/*

fun hello(a: int, b: float): int [
    var num = 10;


    if (a + 1 == 7) [
        num = 11;
        while (1 == 2) [
            var num = 3;

            var op: float = 0.0;

            if (num == 2) [
                op = 98.0;
            ]


                op = 45.1;

        ]
        num =22;
    ] else if (a >= 0) [
        num = 33;
        return 1;
    ] else if ( a - 3 != 2) [
        var tt = 1234;
    ]
    else [
        num = 1;
    ]

    num = 44;
    return num;
]



fun hello(a: int, b: float): int [
    var num = 10;

    for (var i = 0; i < num; i += 1) [
        if (a + 1 == 7) [
            num = 11;
            while (1 == 2) [
                var op: float = 0.0;
                if (num == 2) [
                    return num;
                    op = 98.0;
                ] else [
                    op = 45.1;
                    return num;
                ]
                var num = 3;
                return num;
            ]
            num =22;
        ] else if (a >= 0) [
            num = 33;
            return num;
        ] else if ( a - 3 != 2) [
            var tt = 1234;
            return num;
        ]
        else [
            num = 1;
            return num;
        ]
    ]

    num = 44;
    return num;
]

 */
