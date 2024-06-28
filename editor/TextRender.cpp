#include "TextRender.hpp"

#include <iostream>

#include <glad/glad.h>

#include "Context.hpp"
#include "utils.hpp"

jed::TextRender::TextRender(int width, int height, int x, int y, Font* font)
    : m_width(width)
    , m_height(height)
    , m_x(x)
    , m_y(y)
    , m_font(font)
{
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
    glBindBuffer(GL_ARRAY_BUFFER, m_font->m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void jed::TextRender::draw_char(char c, float x, float y, float scale, glm::vec2& scorll_offset)
{
    Font::Character charachter = m_font->m_charachters[c];

    float xpos = x + charachter.bearing.x * scale + scorll_offset.x;
    float ypos = y - (charachter.size.y - charachter.bearing.y) * scale + scorll_offset.y;

    // THe -5.0f is to allow charachters like \", <, >, etc to be visible on the first line, otherwise the would fail the below test and won't be rendered
    if (!(ypos - 5.0f <= Context::get().height - m_y - Context::get().font_size && ypos >= Context::get().height - m_y - m_height)) {
        return;
    }
    if ((xpos < m_x || xpos >= m_x + m_width)) {
        return;
    }

    float w = charachter.size.x * scale;
    float h = charachter.size.y * scale;
    draw_texture(xpos, ypos, w, h, charachter.texture_id);
}

void jed::TextRender::render_text(Shader& shader, std::string& text, float x, float y, float scale, glm::vec3 color)
{
    shader.use();
    shader.set_uniform_vec("text_color", color);
    shader.set_uniform_matrix("projection", Context::get().projection);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_font->m_vao);

    const float original_x = x;

    for (auto c : text) {
        Font::Character charachter = m_font->m_charachters[c];

        if (c == '\n') {
            y -= Context::get().font_size;
            x = original_x;
            continue;
        } else if (c == '\t') {
            x += (m_font->m_charachters[' '].advance >> 6) * scale * Context::get().tab_width;
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

void jed::TextRender::render_text(Shader& shader, TextData& text, glm::vec2& scroll_offset, float scale, glm::vec3 color)
{
    shader.use();
    shader.set_uniform_vec("text_color", color);
    shader.set_uniform_matrix("projection", Context::get().projection);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_font->m_vao);

    float x = m_x;
    float y = Context::get().height - m_y;
    int line_num = 1;
    y -= Context::get().font_size * scale;

    for (int line = 0; line < text.get_line_count(); line++) {
        x = m_x;

        for (int i = 0; i < text.m_data[line].size; i++) {
            char c = text.m_data[line].data[i];
            draw_char(c, x, y, scale, scroll_offset);
            x += (m_font->m_charachters[c].advance >> 6) * scale;

            if (x + scroll_offset.x > m_x + m_width) {
                break;
            }
        }

        y -= Context::get().font_size;
        x = m_x;

        if ((y + scroll_offset.y + 10.1f) <= (Context::get().height - m_height - m_y) * scale) {
            break;
        }
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    check_for_opengl_error();
}

void jed::TextRender::render_cursor(Shader& m_shader, Cursor cursor, glm::vec2& scroll_offset, glm::vec3& color)
{
    m_shader.use();
    m_shader.set_uniform_vec("text_color", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_font->m_vao);

    const int offset_from_top = (Context::get().height - m_y - Context::get().font_size) - (cursor.line * Context::get().font_size) + scroll_offset.y;
    const int offset_from_left = m_x + cursor.loc * m_font->m_cursor_advance + scroll_offset.x;
    const float cursor_width = 2.0f;
    const float cursor_height = Context::get().font_size;

    if (!(offset_from_top <= Context::get().height - m_y - Context::get().font_size && offset_from_top >= Context::get().height - m_y - m_height)) {
        return;
    }
    if ((offset_from_left < m_x || offset_from_left >= m_x + m_width)) {
        return;
    }

    draw_texture(offset_from_left, offset_from_top, cursor_width, cursor_height, m_font->m_cursor_texture);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    check_for_opengl_error();
}

glm::mat4& jed::TextRender::get_projection()
{
    return Context::get().projection;
}
