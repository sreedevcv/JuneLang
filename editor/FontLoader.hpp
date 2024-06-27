#pragma once

#include <vector>

#include "Font.hpp"

namespace jed {

class FontLoader {
public:
    static FontLoader* get();
    Font*& font(const char* name);

    static void load_fonts();

private:
    FontLoader() = default;
    ~FontLoader() = default;

    static std::map<const char*, Font*> m_font_map;

    static FontLoader font_loader;
};

} // namespace jed
