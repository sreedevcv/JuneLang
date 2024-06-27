#pragma once

// #include <ft2build.h>
// #include FT_FREETYPE_H
#include <glm/glm.hpp>

// #include <map>

#include "Shader.hpp"
#include "TextData.hpp"
#include "Font.hpp"

namespace jed {

class TextRender {
public:
    TextRender(int m_width, int m_height, int x, int y, Font* font);
    ~TextRender() = default;

    void render_text(Shader& shader, std::string& text, float x, float y, float scale, glm::vec3 color);
    void render_text(Shader& shader, TextData& text, glm::vec2& scroll_offset, float scale, glm::vec3 color);
    void render_cursor(Shader& m_shader, Cursor cursor, glm::vec2& scroll_offset, glm::vec3& color);
    glm::mat4& get_projection();

private:
    // std::map<char, Font::Character> m_charachters;
    // FT_Library m_ft;
    // FT_Face m_face;

    Font *m_font;

    unsigned int m_tab_width = 4;
    // unsigned int m_cursor_texture;

    int char_width = 10;
    int char_height = 10;

    int m_width = 100;
    int m_height = 100;
    int m_x = 100;
    int m_y = 100;

    void draw_texture(float xpos, float ypos, float w, float h, unsigned int texture_id);
    void draw_char(char c, float x, float y, float scale, glm::vec2& scorll_offset);
};

} // namespace jed
