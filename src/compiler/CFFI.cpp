#include "CFFI.hpp"
#include "Operand.hpp"
#include "Utils.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <dlfcn.h>
#include <ffi-x86_64.h>
#include <ffi.h>
#include <print>
#include <vector>

jl::CFFI::CFFI(const std::string& lib_path)
    : m_lib_path(lib_path)
{
    m_handle = dlopen(m_lib_path.c_str(), RTLD_LAZY);

    if (m_handle == nullptr) {
        std::println("RUNTIME ERROR: : Unable to open {}", lib_path);
        std::exit(1);
    }
}

jl::CFFI::~CFFI()
{
    if (m_handle != nullptr) {
        dlclose(m_handle);
    }
}

void* get_operand_data(const jl::Operand& operand, void* data_section_offset)
{
    const auto op_type = jl::get_type(operand);
    void* data;
    switch (op_type) {
    case jl::OperandType::NIL:
    case jl::OperandType::BOOL_PTR:
    case jl::OperandType::TEMP:
    case jl::OperandType::UNASSIGNED:
    case jl::OperandType::PTR:
        unimplemented();
        break;
    case jl::OperandType::INT: {
        const auto size = jl::size_of_type(op_type);
        data = malloc(size);
        const auto op_data = std::get<jl::int_type>(operand);
        *((jl::int_type*)data) = op_data;
    } break;
    case jl::OperandType::FLOAT: {
        const auto size = jl::size_of_type(op_type);
        data = malloc(size);
        const auto op_data = std::get<jl::float_type>(operand);
        *((jl::float_type*)data) = op_data;
    } break;
    case jl::OperandType::BOOL: {
        const auto size = jl::size_of_type(op_type);
        data = malloc(size);
        const auto op_data = std::get<bool>(operand);
        *((bool*)data) = op_data;
    } break;
    case jl::OperandType::CHAR: {
        const auto size = jl::size_of_type(op_type);
        data = malloc(size);
        const auto op_data = std::get<char>(operand);
        *((char*)data) = op_data;
    } break;
    case jl::OperandType::CHAR_PTR:
    case jl::OperandType::INT_PTR:
    case jl::OperandType::FLOAT_PTR: {
        const auto size = jl::size_of_type(op_type);
        data = malloc(size);
        const uint64_t mem_offset = (uint64_t)(std::get<jl::PtrVar>(operand).offset + (uint8_t*)(data_section_offset));
        *((uint64_t*)data) = mem_offset;
    } break;
    }

    return data;
}

// void* allocate_space(const jl::Operand& operand)
// {
//     const auto op_type = jl::get_type(operand);
//     void* data;
//     switch (op_type) {
//     case jl::OperandType::PTR:
//     case jl::OperandType::UNASSIGNED:
//     case jl::OperandType::TEMP:
//         unimplemented();
//     case jl::OperandType::INT:
//     case jl::OperandType::FLOAT:
//     case jl::OperandType::NIL:
//     case jl::OperandType::BOOL:
//     case jl::OperandType::CHAR:
//     case jl::OperandType::CHAR_PTR:
//     case jl::OperandType::INT_PTR:
//     case jl::OperandType::FLOAT_PTR:
//     case jl::OperandType::BOOL_PTR:
//         const auto size = jl::size_of_type(op_type);
//         data = malloc(size);
//         break;
//     }

//     // return ;
// }

jl::Operand jl::CFFI::call(
    const std::string& func_name,
    const std::span<Operand> args,
    OperandType return_type,
    void* data_section_offset)
{
    // Load function symbol
    void* func_ptr = dlsym(m_handle, func_name.c_str());
    if (func_ptr == nullptr) {
        std::println("RUNTIME ERROR: : Unable to find symbol {} in {}", func_name, m_lib_path);
        std::exit(1);
    }

    // Prepare the cffi
    ffi_cif cif;
    std::vector<void*> arg_values;

    for (auto& arg : args) {
        arg_values.push_back(get_operand_data(arg, data_section_offset));
    }

    std::vector<ffi_type*> arg_types;
    for (auto& arg : args) {
        auto& c_type = m_june_to_c_types.at(get_type(arg));
        arg_types.push_back(&c_type);
    }

    ffi_type* c_return_type = &m_june_to_c_types.at(return_type);
    unsigned int num_args = args.size();

    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, num_args, c_return_type, arg_types.data()) != FFI_OK) {
        std::println("RUNTIME ERROR: : Unable to prepare FFI for symbol {} in {}", func_name, m_lib_path);
        std::exit(1);
    }

    // Allocate the return value
    size_t ret_val_size = return_type == OperandType::NIL
        ? 8
        : size_of_type(return_type);

    void* ret_val = malloc(ret_val_size);

    // Call the c function
    ffi_call(&cif, (void (*)())(func_ptr), ret_val, arg_values.data());

    // Convert the returned pointer back to an operand
    Operand ret;

    switch (return_type) {
    case OperandType::TEMP:
    case OperandType::UNASSIGNED:
    case OperandType::PTR:
        unimplemented();
    case OperandType::NIL:
        ret = Operand { Nil {} };
        break;
    case OperandType::INT:
        ret = Operand { *(int_type*)ret_val };
        break;
    case OperandType::FLOAT:
        ret = Operand { *(float_type*)ret_val };
        break;
    case OperandType::BOOL:
        ret = Operand { *(bool*)ret_val };
        break;
    case OperandType::CHAR:
        ret = Operand { *(char*)ret_val };
        break;
    case OperandType::CHAR_PTR:
    case OperandType::INT_PTR:
    case OperandType::FLOAT_PTR:
    case OperandType::BOOL_PTR:
        ret = Operand { PtrVar { *(ptr_type*)ret_val } };
        break;
    }

    // Delete allocated values
    for (const auto ptr: arg_values) {
        free(ptr);
    }

    free(ret_val);


    return ret;
}
