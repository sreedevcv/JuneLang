#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include <map>

#include "Shader.hpp"
#include "TextData.hpp"

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
    void render_text(Shader &shader, TextData& text, float x, float y, float scale, glm::vec3 color);
    void render_cursor(Shader& m_shader, Cursor cursor);

private:
    std::map<char, Character> m_charachters;
    FT_Library m_ft;
    FT_Face m_face;

    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_font_size = 24;
    unsigned int m_tab_width = 4;
    unsigned int m_cursor_texture;

    void draw_texture(float xpos, float ypos, float w, float h, unsigned int texture_id);

};

} // namespace jed
