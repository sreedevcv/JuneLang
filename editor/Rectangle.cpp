#include "Rectangle.hpp"

#include <cstdint>

#include <glad/glad.h>

#include "utils.hpp"

jed::Rectangle::Rectangle(float width, float height, float x, float y, glm::vec3 color)
    : m_width(width)
    , m_height(height)
    , m_x(x)
    , m_y(y)
    , m_color(color)
{
}

jed::Rectangle::Rectangle()
{
}

void jed::Rectangle::load()
{
    /* Create the texture for cursor */
    uint8_t cursor_texture_data[] = {
        255,
        255,
        255,
        255,
    };

    int length = 2;
    int breadth = 2;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, length, breadth, 0, GL_RED, GL_UNSIGNED_BYTE, cursor_texture_data);

    float vertices[6][4] = {
        { m_x, m_y + m_height, 0.0f, 0.0f },
        { m_x, m_y, 0.0f, 1.0f },
        { m_x + m_width, m_y, 1.0f, 1.0f },

        { m_x, m_y + m_height, 0.0f, 0.0f },
        { m_x + m_width, m_y, 1.0f, 1.0f },
        { m_x + m_width, m_y + m_height, 1.0f, 0.0f },
    };

    // Generate vao and vbo
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    check_for_opengl_error();
}

void jed::Rectangle::draw(Shader& shader, glm::mat4& projection)
{
    shader.use();
    shader.set_uniform_vec("text_color", m_color);
    shader.set_uniform_matrix("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    check_for_opengl_error();
}
