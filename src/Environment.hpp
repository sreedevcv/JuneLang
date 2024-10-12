#pragma once

#include "Ref.hpp"
#include "Token.hpp"

#include <unordered_map>

namespace jl {

class MemoryPool;

class Environment : public Ref {
public:
    Environment(std::string& file_name);
    Environment(Environment* enclosing);
    virtual ~Environment();

    /* Stores a copy of variable name and value in map if
        they dont already exists otherwise throws an exception */
    void define(const std::string& name, const JlValue& value);
    /* Retrives the sored reference to a token otherwise
        throws an exception */
    JlValue& get(Token& name);
    JlValue& get_at(Token& name, int depth);
    JlValue& get_at(std::string& name, int depth);
    void assign(Token& token, JlValue& value);
    void assign(Token& token, JlValue&& value);
    void assign_at(Token& token, JlValue& value, int depth);
    Environment* ancestor(int depth);

    Environment* m_enclosing;

private:
    std::unordered_map<std::string, JlValue> m_values;
    std::string& m_file_name;

    friend class MemoryPool;
};

} // namespace jl
