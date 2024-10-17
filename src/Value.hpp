#pragma once

#include "Ref.hpp"
#include <string>
#include <variant>
#include <print>
#include <vector>

// #include "Wrapper.hpp"

namespace jl {

class Instance;
class Callable;
class Expr;

struct JlValue : public Ref {
    virtual ~JlValue() = default;
    typedef enum {
        NONE,
        INT,
        FLOAT,
        STR,
        BOOL,
        CALL,
        OBJ,
        LIST,
        JNULL,
    } Type;

    Type m_type = NONE;
    virtual JlValue* to();
};

struct JlInt : public JlValue {
    int m_val;
    JlInt(int val);
    virtual ~JlInt() = default;
};

struct JlFloat : public JlValue {
    double m_val;
    JlFloat(double val);
    virtual ~JlFloat() = default;
};

struct JlBool : public JlValue {
    bool m_val;
    JlBool(bool val);
    virtual ~JlBool() = default;
};

struct JlStr : public JlValue {
    std::string m_val;
    JlStr(const std::string& val);
    virtual ~JlStr() = default;
};

struct JlCallable : public JlValue {
    Callable* m_val;
    JlCallable(Callable* val);
    virtual ~JlCallable() = default;
};

struct JlObj : public JlValue {
    Instance* m_val;
    JlObj(Instance* val);
    virtual ~JlObj() = default;
};

struct JlList : public JlValue {
    std::vector<Expr*>* m_val;
    JlList(std::vector<Expr*>* val);
    virtual ~JlList() = default;
};

struct JlNull : public JlValue {
    JlNull();
    virtual ~JlNull() = default;
};

namespace is {
    bool _int(JlValue* ref);
    bool _float(JlValue* ref);
    bool _bool(JlValue* ref);
    bool _str(JlValue* ref);
    bool _callable(JlValue* ref);
    bool _obj(JlValue* ref);
    bool _list(JlValue* ref);
    bool _number(JlValue* ref);
    bool _null(JlValue* ref);
    bool _same(JlValue* ref1, JlValue* ref2);
    /* User needs to check they are of the same type before using */
    bool _exact_same(JlValue* ref1, JlValue* ref2);
}

template<typename T>
T vget(JlValue* ref)
{
    std::println("Called jl::vget with un-supported template type");
    std::exit(1);
}

template<>
inline int vget(JlValue* ref)
{
    if (dynamic_cast<JlInt*>(ref)) {
        return static_cast<JlInt*>(ref)->m_val;
    }
    else {
        std::println("Failed to call jl::vget for int");
        std::exit(1);
    }
}

template<>
inline double vget(JlValue* ref)
{
    if (dynamic_cast<JlFloat*>(ref)) {
        return static_cast<JlFloat*>(ref)->m_val;
    }
    else {
        std::println("Failed to call jl::vget for double");
        std::exit(1);
    }
}

template<>
inline bool vget(JlValue* ref)
{
    if (dynamic_cast<JlBool*>(ref)) {
        return static_cast<JlBool*>(ref)->m_val;
    }
    else {
        std::println("Failed to call jl::vget for bool");
        std::exit(1);
    }
}

template<>
inline std::string& vget(JlValue* ref)
{
    if (dynamic_cast<JlStr*>(ref)) {
        return static_cast<JlStr*>(ref)->m_val;
    }
    else {
        std::println("Failed to call jl::vget for std::string&");
        std::exit(1);
    }
}

template<>
inline std::string vget(JlValue* ref)
{
    if (dynamic_cast<JlStr*>(ref)) {
        return static_cast<JlStr*>(ref)->m_val;
    }
    else {
        std::println("Failed to call jl::vget for std::string");
        std::exit(1);
    }
}

template<>
inline Callable* vget(JlValue* ref)
{
    if (dynamic_cast<JlCallable*>(ref)) {
        return dynamic_cast<JlCallable*>(ref)->m_val;
    }
    else {
        std::println("Failed to call jl::vget for Callable*");
        std::exit(1);
    }
}

template<>
inline Instance* vget(JlValue* ref)
{
    if (dynamic_cast<JlObj*>(ref)) {
        return static_cast<JlObj*>(ref)->m_val;
    }
    else {
        std::println("Failed to call jl::vget for Instance*");
        std::exit(1);
    }
}

template<>
inline std::vector<Expr*>* vget(JlValue* ref)
{
    if (dynamic_cast<JlList*>(ref)) {
        return static_cast<JlList*>(ref)->m_val;
    }
    else {
        std::println("Failed to call jl::vget for std::vector<Expr*>*");
        std::exit(1);
    }
}

//template<typename T>
//auto get(JlValue* ref)
//{
//    return static_cast<T*>(ref)->m_val;
//}


} // namespace jl
