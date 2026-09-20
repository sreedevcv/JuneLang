#pragma once

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

    static std::map<std::string_view, Font*> m_font_map;

    static FontLoader font_loader;
};

} // namespace jed
