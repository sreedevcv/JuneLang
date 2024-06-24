#include "TextRender.hpp"

#include <iostream>

#include <glad/glad.h>

#include "Context.hpp"
#include "utils.hpp"

jed::TextRender::TextRender(int width, int height)
    : m_width(width)
    , m_height(height)
    , projection(glm::ortho(0.0f, static_cast<float>(m_width), 0.0f, static_cast<float>(m_height)))
{
    /* Initalize and load the font using freetype */
    if (FT_Init_FreeType(&m_ft)) {
        std::cout << "Freetype Error: Could not init FreeType Library" << std::endl;
        std::exit(-1);
    }

    if (FT_New_Face(m_ft, "./res/fonts/CascadiaMono.ttf", 0, &m_face)) {
        std::cout << "Freetype Error: Could not load font" << std::endl;
        std::exit(-1);
    }

    std::cout << "Font loaded" << std::endl;

    FT_Set_Pixel_Sizes(m_face, 0, Context::get().font_size);
    std::cout << m_width << " " << m_height << std::endl;
}

void jed::TextRender::load_fonts()
{
    check_for_opengl_error();

    /* Create the texture for cursor */
    uint8_t cursor_texture_data[] = {
        255,
        255,
        255,
        255,
    };

    glGenTextures(1, &m_cursor_texture);
    glBindTexture(GL_TEXTURE_2D, m_cursor_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 2, 2, 0, GL_RED, GL_UNSIGNED_BYTE, cursor_texture_data);

    // Generate vao and vbo
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    check_for_opengl_error();

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(m_face, c, FT_LOAD_RENDER)) {
            std::cout << "Freetype Error: Could not load glyph: " << c << std::endl;
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, m_face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            .texture_id = texture,
            .size = glm::ivec2(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows),
            .bearing = glm::ivec2(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top),
            .advance = static_cast<unsigned int>(m_face->glyph->advance.x)
        };
        m_charachters[c] = character;

        m_cursor_advance = (character.advance >> 6);
    }

    check_for_opengl_error();

    FT_Done_Face(m_face);
    FT_Done_FreeType(m_ft);
}

void jed::TextRender::draw_texture(float xpos, float ypos, float w, float h, unsigned int texture_id)
{
    float vertices[6][4] = {
        { xpos, ypos + h, 0.0f, 0.0f },
        { xpos, ypos, 0.0f, 1.0f },
        { xpos + w, ypos, 1.0f, 1.0f },

        { xpos, ypos + h, 0.0f, 0.0f },
        { xpos + w, ypos, 1.0f, 1.0f },
        { xpos + w, ypos + h, 1.0f, 0.0f },
    };

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void jed::TextRender::draw_char(char c, float x, float y, float scale)
{
    Character charachter = m_charachters[c];

    float xpos = x + charachter.bearing.x * scale;
    float ypos = y - (charachter.size.y - charachter.bearing.y) * scale;

    float w = charachter.size.x * scale;
    float h = charachter.size.y * scale;
    draw_texture(xpos, ypos, w, h, charachter.texture_id);
}

void jed::TextRender::render_text(Shader& shader, std::string& text, float x, float y, float scale, glm::vec3 color)
{
    shader.use();
    shader.set_uniform_vec("text_color", color);
    shader.set_uniform_matrix("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_vao);

    const float original_x = x;

    for (auto c : text) {
        Character charachter = m_charachters[c];

        if (c == '\n') {
            y -= Context::get().font_size;
            x = original_x;
            continue;
        } else if (c == '\t') {
            x += (m_charachters[' '].advance >> 6) * scale * Context::get().tab_width;
            continue;
        }

        float xpos = x + charachter.bearing.x * scale;
        float ypos = y - (charachter.size.y - charachter.bearing.y) * scale;

        float w = charachter.size.x * scale;
        float h = charachter.size.y * scale;

        draw_texture(xpos, ypos, w, h, charachter.texture_id);
        x += (charachter.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void jed::TextRender::render_text(Shader& shader, TextData& text, float x, float y, float scale, glm::vec3 color)
{
    shader.use();
    shader.set_uniform_vec("text_color", color);
    shader.set_uniform_matrix("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_vao);

    const float original_x = x;
    int line_num = 1;

    for (int line = 0; line < text.get_line_count(); line++) {
        std::string num = std::to_string(line_num++);

        for (char c : num) {
            draw_char(c, x, y, scale);
            x += (m_charachters[c].advance >> 6) * scale;
        }

        x = original_x;
        for (int i = 0; i < text.m_data[line].size; i++) {
            char c = text.m_data[line].data[i];
            /* Offset the x-axis with the gutter width */
            draw_char(c, x + Context::get().gutter_width, y, scale);
            x += (m_charachters[c].advance >> 6) * scale;
        }

        y -= Context::get().font_size;
        x = original_x;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    check_for_opengl_error();
}

void jed::TextRender::render_cursor(Shader& m_shader, Cursor cursor, float delta)
{
    m_shader.use();
    m_shader.set_uniform_vec("text_color", glm::vec3(0.4f, 0.4f, 0.4f));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_vao);

    const int offset_from_top = (m_height - Context::get().font_size) - cursor.line * Context::get().font_size;
    const int offset_from_left = cursor.loc * m_cursor_advance;
    const float cursor_width = 2.0f;
    const float cursor_height = Context::get().font_size;

    draw_texture(offset_from_left + Context::get().gutter_width, offset_from_top, cursor_width, cursor_height, m_cursor_texture);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    check_for_opengl_error();
}

glm::mat4& jed::TextRender::get_projection()
{
    return projection;
}
