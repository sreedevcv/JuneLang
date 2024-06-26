#pragma once

#include <glm/glm.hpp>

#include "Shader.hpp"

namespace jed {

class Rectangle {
public:
    Rectangle(float width, float height, float x, float y, glm::vec3 color);
    Rectangle();

    void load();
    void draw(Shader& shader, glm::mat4& projection);

private:
    float m_width;
    float m_height;
    float m_x;
    float m_y;

    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int texture;

    glm::vec3 m_color;
};

} // namespace jed
