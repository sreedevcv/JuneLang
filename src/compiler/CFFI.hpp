#pragma once

#include <ffi.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "Operand.hpp"
#include "Utils.hpp"

namespace jl {
class CFFI {
public:
    CFFI(const std::string& lib_path);
    ~CFFI();

    CFFI(const CFFI& cffi) = delete;
    CFFI(CFFI&& cffi) = delete;
    CFFI& operator=(const CFFI& cffi) = delete;
    CFFI& operator=(CFFI&& cffi) = delete;

    jl::reg_type call(
        const std::string& func_name,
        const std::vector<std::pair<reg_type, OperandType>>& args,
        OperandType return_type);

private:
    std::string m_lib_path;
    void* m_handle { nullptr };
    std::unordered_map<OperandType, ffi_type> m_june_to_c_types {
        { OperandType::INT, ffi_type_sint },
        { OperandType::CHAR, ffi_type_schar },
        { OperandType::BOOL, ffi_type_uint8 },
        { OperandType::FLOAT, sizeof(float_type) == 4 ? ffi_type_float : ffi_type_double },
        { OperandType::INT_PTR, ffi_type_pointer },
        { OperandType::CHAR_PTR, ffi_type_pointer },
        { OperandType::BOOL_PTR, ffi_type_pointer },
        { OperandType::FLOAT_PTR, ffi_type_pointer },
        { OperandType::NIL_PTR, ffi_type_pointer },
        { OperandType::NIL, ffi_type_void },
    };
};
}