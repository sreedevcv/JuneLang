#pragma once

#include "Ref.hpp"
#include <string>
#include <variant>
#include <vector>

// #include "Wrapper.hpp"

namespace jl {
class Instance;
class Callable;
class Expr;

class JNullType {
public:
    inline bool operator==(const JNullType other) const
    {
        return true;
    }
};

struct JlValue : public Ref {
//public:
//    using Value
//        = std::variant<
//            int,
//            double,
//            bool,
//            std::string,
//            JNullType, // Respresents null value
//            Callable*,
//            Instance*,
//            std::vector<Expr*>*>;
//
//    Value m_value;
//
//    JlValue() = default;
//    virtual ~JlValue() = default;
//    
//    JlValue(const Value& value);
//    JlValue(const Value&& value);
//    JlValue(Value* value);
//    size_t index() const;
//    Value get();

    virtual ~JlValue() = default;
};

struct JlInt : public JlValue {
    int m_val;
    JlInt() = default; 
    virtual ~JlInt() = default;
};

struct JlFloat : public JlValue {
    double m_val;
    JlFloat() = default; 
    virtual ~JlFloat() = default;
};

struct JlBool : public JlValue {
    bool m_val;
    JlBool() = default; 
    virtual ~JlBool() = default;
};

struct JlStr : public JlValue {
    std::string m_val;
    JlStr() = default; 
    virtual ~JlStr() = default;
};

struct JlCallable : public JlValue {
    Callable* m_val;
    JlCallable() = default; 
    virtual ~JlCallable() = default;
};

struct JlObj : public JlValue {
    Instance* m_val;
    JlObj() = default; 
    virtual ~JlObj() = default;
};

struct JlList : public JlValue {
    std::vector<Expr*>* m_val;
    JlList() = default;
    virtual ~JlList() = default;
};

namespace is {
    bool _int(JlValue* ref);
    bool _float(JlValue* ref);
    bool _bool(JlValue* ref);
    bool _str(JlValue* ref);
    bool _callable(JlValue* ref);
    bool _obj(JlValue* ref);
    bool _list(JlValue* ref);
}

} // namespace jl
