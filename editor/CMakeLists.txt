# include(AddGLFW_GLM)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG e2c92645460f680fd272fd2eed591efb2be7dc31
)

if (UNIX)
    set(GLFW_USE_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)

    
    # set(GLFW_USE_WAYLAND ON CACHE BOOL "" FORCE)
    # set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)

elseif(WIN32)
    set(GLFW_USE_WAYLAND OFF CACHE BOOL "" FORCE)
endif()
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 33b0eb9fa336ffd8551024b1d2690e418014553b
)
set(GLM_BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(glm)



FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG 044d142be7b6a93b6940367a1bc5847451ff4775
)
FetchContent_MakeAvailable(freetype)

add_library(JuneEditor
    Editor.cpp
    glad.c
    TextRender.cpp
    Shader.cpp
    TextData.cpp
    Timer.cpp
    Rectangle.cpp
    Context.cpp
)

target_link_libraries(JuneEditor PRIVATE glfw glm::glm freetype)
target_include_directories(JuneEditor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})