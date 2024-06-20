#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include <map>

#include "Shader.hpp"

namespace jed {

class TextRender {
public:
    TextRender();
    ~TextRender() = default;

    struct Character {
        unsigned int texture_id;
        glm::ivec2 size;
        glm::ivec2 bearing;
        unsigned int advance;
    };

    void load_fonts();
    void render_text(Shader &shader, std::string& text, float x, float y, float scale, glm::vec3 color);

private:
    std::map<char, Character> m_charachters;
    FT_Library m_ft;
    FT_Face m_face;

    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_font_size = 48;
    unsigned int m_tab_width = 4;
};

} // namespace jed
