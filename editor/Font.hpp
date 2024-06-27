#pragma once

#include <iostream>
#include <map>

#include <ft2build.h>
#include <glm/glm.hpp>
#include FT_FREETYPE_H

namespace jed {

class Font {
public:
    Font(const char* name, int font_size);

    struct Character {
        unsigned int texture_id;
        glm::ivec2 size;
        glm::ivec2 bearing;
        unsigned int advance;
    };

    std::map<char, Character> m_charachters;
    void load();
    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_cursor_texture;
    int m_cursor_advance;

private:
    FT_Library m_ft;
    FT_Face m_face;
    int m_font_size;
};

} // namespace jed
