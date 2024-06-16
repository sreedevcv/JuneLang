#pragma once

#include <unordered_map>

#include "Token.hpp"

namespace jl {

class Environment {
public:
    Environment(std::string& file_name);
    Environment(Environment *enclosing);
    ~Environment();

    /* Stores a copy of variable name and value in map if
        they dont already exists otherwise throws an exception */
    void define(const std::string& name, const Value& value);
    /* Retrives the sored reference to a token otherwise
        throws an exception */
    Value& get_ref( Token& name);
    Value get_copy( Token& name);

    void assign( Token& token,  Value& value);
    Environment* m_enclosing;

private:
    std::unordered_map<std::string, Value> m_values;
    std::string& m_file_name;
};

} // namespace jl
