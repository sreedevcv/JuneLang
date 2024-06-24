#include "Context.hpp"


jed::Context& jed::Context::get()
{
    static Context context;
    return context;
}