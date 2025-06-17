#pragma once

#include "Ref.hpp"
#include "Token.hpp"

#include <unordered_map>
#include <unordered_set>

namespace jl {

class MemoryPool;
class GarbageCollector;

class Environment : public Ref {
public:
    Environment(std::string& file_name);
    Environment(Environment* enclosing);
    virtual ~Environment();

    /* Stores a copy of variable name and value in map if
        they dont already exists otherwise throws an exception */
    void define(const std::string& name, Value* value);
    /* Retrives the sored reference to a token otherwise
        throws an exception */
    Value* get(Token& name);
    Value* get_at(Token& name, int depth);
    Value* get_at(std::string& name, int depth);
    void assign(Token& token, Value* value);
    void assign_at(Token& token, Value* value, int depth);
    Environment* ancestor(int depth);

    Environment* m_enclosing;

private:
    std::unordered_map<std::string, Value*> m_values;
    std::string& m_file_name;
    std::unordered_set<Ref*> m_refs;

    friend class MemoryPool;
    friend class GarbageCollector;
};

} // namespace jl
