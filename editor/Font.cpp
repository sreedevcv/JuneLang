#include "Font.hpp"

#include <glad/glad.h>

#include "utils.hpp"

jed::Font::Font(const char* name, int font_size)
    : m_font_size(font_size)
{
    /* Initalize and load the font using freetype */
    if (FT_Init_FreeType(&m_ft)) {
        std::cout << "Freetype Error: Could not init FreeType Library" << std::endl;
        std::exit(-1);
    }

    if (FT_New_Face(m_ft, name, 0, &m_face)) {
        std::cout << "Freetype Error: Could not load font" << std::endl;
        std::exit(-1);
    }

    std::cout << "Font loaded" << std::endl;

    FT_Set_Pixel_Sizes(m_face, 0, font_size);
}

void jed::Font::load()
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
