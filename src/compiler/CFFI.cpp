#include "CFFI.hpp"

#include <cstdlib>
#include <dlfcn.h>
#include <ffi-x86_64.h>
#include <ffi.h>
#include <print>

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

jl::reg_type jl::CFFI::call(
    const std::string& func_name,
    const std::vector<std::pair<reg_type, OperandType>>& args,
    OperandType return_type)
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

    std::vector<ffi_type*> arg_types;
    for (auto& [arg, type] : args) {
        arg_values.push_back((void*)&(arg));
        auto& c_type = m_june_to_c_types.at(type);
        arg_types.push_back(&c_type);
    }


    ffi_type* c_return_type = &m_june_to_c_types.at(return_type);
    unsigned int num_args = args.size();

    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, num_args, c_return_type, arg_types.data()) != FFI_OK) {
        std::println("RUNTIME ERROR: : Unable to prepare FFI for symbol {} in {}", func_name, m_lib_path);
        std::exit(1);
    }

    reg_type ret_val;

    // Call the c function
    ffi_call(&cif, (void (*)())(func_ptr), &ret_val, arg_values.data());

    return ret_val;
}
