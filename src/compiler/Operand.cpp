#include "Operand.hpp"

#include "Utils.hpp"
#include <optional>
#include <string>

std::string jl::to_string(const Operand& operand)
{
    switch (get_type(operand)) {
    case OperandType::INT:
        return std::to_string(std::get<int>(operand));
    case OperandType::FLOAT:
        return std::to_string(std::get<double>(operand));
    case OperandType::TEMP:
        return "[" + std::to_string(std::get<TempVar>(operand).idx) + "]";
    case OperandType::NIL:
        return "Nil";
    case OperandType::BOOL:
        return (std::get<bool>(operand) == true) ? "true" : "false";
    case OperandType::UNASSIGNED:
        return "UNKNOWN";
    case OperandType::CHAR: {
        std::string ch { '\'', std::get<char>(operand), '\'' };
        return ch;
    }
    case OperandType::CHAR_PTR:
        return "C(" + std::to_string(std::get<PtrVar>(operand).offset) + ")";
    case OperandType::INT_PTR:
        return "I(" + std::to_string(std::get<PtrVar>(operand).offset) + ")";
    case OperandType::FLOAT_PTR:
        return "F(" + std::to_string(std::get<PtrVar>(operand).offset) + ")";
    case OperandType::BOOL_PTR:
        return "B(" + std::to_string(std::get<PtrVar>(operand).offset) + ")";
    case OperandType::NIL_PTR:
        return "N(" + std::to_string(std::get<PtrVar>(operand).offset) + ")";
    }

    unimplemented();
    return "Unimplemented";
}

jl::OperandType jl::get_type(const Operand& operand)
{
    switch (operand.index()) {
    case 0:
        return OperandType::INT;
    case 1:
        return OperandType::FLOAT;
    case 2:
        return OperandType::TEMP;
    case 3:
        return OperandType::NIL;
    case 4:
        return OperandType::BOOL;
    case 5:
        return OperandType::CHAR;
    case 6:
        return std::get<PtrVar>(operand).type;
    }

    unimplemented();
    return OperandType::NIL;
}

std::string jl::to_string(const OperandType& type)
{
    switch (type) {
    case OperandType::INT:
        return "INT";
    case OperandType::FLOAT:
        return "FLOAT";
    case OperandType::TEMP:
        return "TEMP";
    case OperandType::NIL:
        return "NIL";
    case OperandType::BOOL:
        return "BOOL";
    case OperandType::UNASSIGNED:
        return "UNASSIGNED";
    case OperandType::CHAR:
        return "CHAR";
    case OperandType::CHAR_PTR:
        return "CHAR_PTR";
    case OperandType::BOOL_PTR:
        return "BOOL_PTR";
    case OperandType::INT_PTR:
        return "INT_PTR";
    case OperandType::FLOAT_PTR:
        return "FLOAT_PTR";
    case OperandType::NIL_PTR:
        return "PTR";
    }

    unimplemented();
    return "UNIMPLEMENTED";
}

bool jl::is_number(const Operand& operand)
{
    const auto type = get_type(operand);
    return type == OperandType::INT || type == OperandType::FLOAT;
}

bool jl::is_number(const OperandType type)
{
    return type == OperandType::INT || type == OperandType::FLOAT;
}

bool jl::is_ptr(const OperandType type)
{
    return type == OperandType::CHAR_PTR
        || type == OperandType::INT
        || type == OperandType::FLOAT_PTR
        || type == OperandType::INT_PTR
        || type == OperandType::BOOL_PTR;
}

bool jl::is_pure_ptr(const OperandType type)
{
    return type == OperandType::CHAR_PTR
        || type == OperandType::FLOAT_PTR
        || type == OperandType::INT_PTR
        || type == OperandType::NIL_PTR
        || type == OperandType::BOOL_PTR;
}

std::optional<jl::OperandType> jl::from_str(const std::string& type_name)
{
    if (type_name == "int") {
        return OperandType::INT;
    } else if (type_name == "float") {
        return OperandType::FLOAT;
    } else if (type_name == "bool") {
        return OperandType::BOOL;
    } else if (type_name == "nil") {
        return OperandType::NIL;
    } else if (type_name == "char") {
        return OperandType::CHAR;
    } else if (type_name == "char_ptr") {
        return OperandType::CHAR_PTR;
    } else if (type_name == "float_ptr") {
        return OperandType::FLOAT_PTR;
    } else if (type_name == "int_ptr") {
        return OperandType::INT_PTR;
    } else if (type_name == "bool_ptr") {
        return OperandType::BOOL_PTR;
    } else {
        return std::nullopt;
    }
}

jl::Operand jl::default_operand(OperandType type)
{
    switch (type) {
    case OperandType::INT:
        return int {};
    case OperandType::FLOAT:
        return double {};
    case OperandType::TEMP:
        return TempVar { 0 };
    case OperandType::NIL:
        return Nil {};
    case OperandType::BOOL:
        return bool {};
    case jl::OperandType::CHAR:
        return '\0';
    case OperandType::UNASSIGNED:
        return Nil {};
    case OperandType::CHAR_PTR:
        return PtrVar { .offset = 0, .type = OperandType::CHAR_PTR };
    case OperandType::BOOL_PTR:
        return PtrVar { .offset = 0, .type = OperandType::BOOL_PTR };
    case OperandType::INT_PTR:
        return PtrVar { .offset = 0, .type = OperandType::INT_PTR };
    case OperandType::FLOAT_PTR:
        return PtrVar { .offset = 0, .type = OperandType::FLOAT_PTR };
    case OperandType::NIL_PTR:
        break;
    }

    unimplemented();
    return Nil {};
}

// Convert a primitive type to its ptr variant
std::optional<jl::OperandType> jl::into_ptr(OperandType type)
{
    switch (type) {
    case OperandType::CHAR:
        return OperandType::CHAR_PTR;
    case OperandType::INT:
        return OperandType::INT_PTR;
    case OperandType::FLOAT:
        return OperandType::FLOAT_PTR;
    case OperandType::BOOL:
        return OperandType::BOOL_PTR;
    case OperandType::NIL:
        return OperandType::NIL_PTR;

    case OperandType::TEMP:
    case OperandType::UNASSIGNED:
    case OperandType::NIL_PTR:
    case OperandType::CHAR_PTR:
    case OperandType::INT_PTR:
    case OperandType::FLOAT_PTR:
    case OperandType::BOOL_PTR:
        return std::nullopt;
    }
}

// Convert a ptr type to its base variant
std::optional<jl::OperandType> jl::from_ptr(OperandType type)
{
    switch (type) {
    case OperandType::CHAR_PTR:
        return OperandType::CHAR;
    case OperandType::INT_PTR:
        return OperandType::INT;
    case OperandType::FLOAT_PTR:
        return OperandType::FLOAT;
    case OperandType::BOOL_PTR:
        return OperandType::BOOL;
    case OperandType::NIL_PTR:
        return OperandType::NIL;

    case OperandType::TEMP:
    case OperandType::NIL:
    case OperandType::UNASSIGNED:
    case OperandType::INT:
    case OperandType::FLOAT:
    case OperandType::BOOL:
    case OperandType::CHAR:
        return std::nullopt;
    }
}

size_t jl::size_of_type(OperandType type)
{
    switch (type) {
    case OperandType::TEMP:
    case OperandType::NIL:
    case OperandType::UNASSIGNED:
        unimplemented();

    case OperandType::INT:
        return sizeof(int_type);
    case OperandType::FLOAT:
        return sizeof(float_type);
    case OperandType::BOOL:
        return sizeof(bool);
    case OperandType::CHAR:
        return sizeof(char);
    case OperandType::CHAR_PTR:
    case OperandType::NIL_PTR:
    case OperandType::INT_PTR:
    case OperandType::FLOAT_PTR:
    case OperandType::BOOL_PTR:
        return sizeof(ptr_type);
    }
}

std::optional<jl::OperandType> jl::from_typeinfo(const TypeInfo& type_info)
{
    if (!type_info.is_array) {
        if (type_info.name == "int") {
            return OperandType::INT;
        } else if (type_info.name == "float") {
            return OperandType::FLOAT;
        } else if (type_info.name == "bool") {
            return OperandType::BOOL;
        } else if (type_info.name == "nil") {
            return OperandType::NIL;
        } else if (type_info.name == "char") {
            return OperandType::CHAR;
        }
    } else {
        if (type_info.name == "int") {
            return OperandType::INT_PTR;
        } else if (type_info.name == "float") {
            return OperandType::FLOAT_PTR;
        } else if (type_info.name == "bool") {
            return OperandType::BOOL_PTR;
        } else if (type_info.name == "nil") {
            return OperandType::NIL_PTR;
        } else if (type_info.name == "char") {
            return OperandType::CHAR_PTR;
        }
    }

    return std::nullopt;
}