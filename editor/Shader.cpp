#include "Shader.hpp"

Shader::Shader(const char* vertShaderSource, const char* fragShaderSource, const char* geom_shader_source)
    : mVert_shader_path(vertShaderSource)
    , mFrag_shader_path(fragShaderSource)
    , mHasCompiled(false)
    , mGeom_shader_path(geom_shader_source)
{
}

Shader::Shader()
{
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

void Shader::compile()
{
    if (mHasCompiled) {
        return;
    }

    mHasCompiled = true;

    std::ifstream vertFile;
    std::ifstream fragFile;

    vertFile.open(mVert_shader_path);
    fragFile.open(mFrag_shader_path);

    std::stringstream vertStream;
    std::stringstream fragStream;
    std::stringstream geomStream;

    vertStream << vertFile.rdbuf();
    fragStream << fragFile.rdbuf();

    std::string vertCode = vertStream.str();
    std::string fragCode = fragStream.str();

    const char* vertCodeStr = vertCode.c_str();
    const char* fragCodeStr = fragCode.c_str();
    const char* geomCodeStr = nullptr;

    if (mGeom_shader_path) {
        std::ifstream geomfile(mGeom_shader_path);
        geomStream << geomfile.rdbuf();
        geomCodeStr = geomStream.str().c_str();
    }

    compile_shader_code(vertCodeStr, fragCodeStr, geomCodeStr);

    check_for_opengl_error();
}

void Shader::compile_shader_code(const char* vertex, const char* fragment, const char* geometry)
{
    const auto getError = [&](unsigned int shader, bool is_vert_shader = true, bool is_program = false) {
        int success;
        char log[1024];
        if (is_program) {
            glGetProgramiv(ID, GL_LINK_STATUS, &success);
        } else {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        }

        if (!success) {
            if (is_program) {
                glGetProgramInfoLog(ID, 1024, nullptr, log);
                std::cout << "[ERROR]::LINK::";
            } else {
                glGetShaderInfoLog(shader, 1024, nullptr, log);
                // std::cout << "[ERROR]::" << (is_vert_shader ? "VERT" : "FRAG") << "::" << mVert_shader_path << "::";
                std::cout << "[ERROR]::";
                if (is_vert_shader) {
                    std::cout << "VERT::\n" << vertex << std::endl; 
                } else {
                    std::cout << "FRAG::\n" << fragment << std::endl; 
                }
            }
            std::cout << log << std::endl;
        }
    };

    unsigned int vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertex, nullptr);
    glCompileShader(vertShader);
    getError(vertShader);

    unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragment, nullptr);
    glCompileShader(fragShader);
    getError(fragShader, false);

    unsigned int geomShader;
    if (geometry) {
        geomShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geomShader, 1, &geometry, nullptr);
        glCompileShader(geomShader);
        getError(geomShader, false);
    }

    ID = glCreateProgram();
    glAttachShader(ID, vertShader);
    glAttachShader(ID, fragShader);
    if (geometry) {
        glAttachShader(ID, geomShader);
    }
    glLinkProgram(ID);
    getError(ID, false, true);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    if (geometry) {
        glDeleteShader(geomShader);
    }
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::set_uniform_float(const char* name, const float value)
{
    int loc = glGetUniformLocation(ID, name);
    glUniform1f(loc, value);
}

void Shader::set_uniform_matrix(const char* name, const glm::mat4& value)
{
    int loc = glGetUniformLocation(ID, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::set_uniform_vec(const char* name, const glm::vec3&& value)
{
    int loc = glGetUniformLocation(ID, name);
    glUniform3fv(loc, 1, glm::value_ptr(value));
}


void Shader::set_uniform_vec(const char* name, const glm::vec4&& value)
{
    int loc = glGetUniformLocation(ID, name);
    glUniform4fv(loc, 1, glm::value_ptr(value));
}

void Shader::set_uniform_vec(const char* name, const glm::vec3& value)
{
    int loc = glGetUniformLocation(ID, name);
    glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Shader::set_uniform_int(const char* name, const int value) const
{
    int loc = glGetUniformLocation(ID, name);
    glUniform1i(loc, value);
}

void Shader::set_uniform_vec(const char* name, const glm::vec4& value)
{
    int loc = glGetUniformLocation(ID, name);
    glUniform4fv(loc, 1, glm::value_ptr(value));
}

void Shader::create_shader_using_files(const char* vert_shader_path, const char* frag_shader_path, const char* geom_shader_path)
{
    mVert_shader_path = vert_shader_path;
    mFrag_shader_path = frag_shader_path;
    mGeom_shader_path = geom_shader_path;
    mHasCompiled = false;
}

void Shader::create_shader_using_source(const char* vert_shader_source, const char* frag_shader_source, const char* geom_shader_source)
{
    compile_shader_code(vert_shader_source, frag_shader_source, geom_shader_source);
    mHasCompiled = true;
}

Shader ShaderManager::simple_shader()
{
    Shader shader("", "");
    shader.mHasCompiled = true;

    const char* vert_shader = R"(
        #version 330
        layout (location = 0) in vec3 apos;

        void main() {
            gl_Position = vec4(apos, 1.0);
        }
    )";

    const char* frag_shader = R"(
        #version 330 core 

        out vec4 frag_color;
        in vec4 color;

        void main() {
            frag_color = color;
        }
    )";

    shader.compile_shader_code(vert_shader, frag_shader);
    return shader;
}

Shader ShaderManager::mvp_shader()
{
    Shader shader("", "");
    shader.mHasCompiled = true;

    const char* vert_shader = R"(
        #version 330
        layout (location = 0) in vec3 apos;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            gl_Position = projection * view * model * vec4(apos, 1.0);
        }
    )";

    const char* frag_shader = R"(
        #version 330 core 

        out vec4 frag_color;
        
        uniform vec4 color;

        void main() {
            frag_color = color;
        }
    )";

    shader.compile_shader_code(vert_shader, frag_shader);
    return shader;
}

Shader ShaderManager::create_shader(const char* vert_code, const char* frag_code, const char *geom_code)
{
    Shader shader("", "");
    shader.mHasCompiled = true;
    shader.compile_shader_code(vert_code, frag_code, geom_code);
    return shader;
}
