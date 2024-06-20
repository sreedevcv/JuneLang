#pragma once

// #define check_for_opengl_error()                                                             \
//     while (GLenum error = glGetError()) {                                                    \
//         std::cout << __FILE__ << ":" << __LINE__ << " OpenGL Error: " << error << std::endl; \
//     }

#include <glad/glad.h>

inline GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            // case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            // case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define check_for_opengl_error() glCheckError_(__FILE__, __LINE__)

struct Color {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
};

consteval float norm_color(int color)
{
    return static_cast<float>(color) / 255.0f;
}

const auto printMat = [](const glm::mat4& m) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << m[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
};

const auto printVec = [](const glm::vec3& m) {
    std::cout << m.x << " " << m.y << " " << m.z << "\n";
};

const auto get_signed_angle = [](const glm::vec3& a, const glm::vec3& b, const glm::vec3& normal) -> float
{
    float angle = glm::acos(glm::dot(a, b));
    if (glm::dot(normal, glm::cross(a, b)) < 0) {
        angle = -angle;
    }
    return angle;
};