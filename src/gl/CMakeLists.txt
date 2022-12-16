cmake_minimum_required(VERSION 3.25)
project(gl LANGUAGES CXX)

add_library(gl STATIC
    application.hpp application.cpp
    mesh.hpp mesh.cpp
    model.hpp model.cpp
    shader.hpp shader.cpp
    camera.hpp camera.cpp
    io.hpp io.cpp
    framebuffer.hpp framebuffer.cpp
    texture.hpp texture.cpp texture.inl
    renderbuffer.hpp renderbuffer.cpp
)

target_link_libraries(gl PUBLIC glad::glad glfw glm::glm imgui::imgui tinyobjloader::tinyobjloader)
target_compile_features(gl PRIVATE cxx_std_20)
set_target_properties(gl PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(gl PUBLIC ${STB_INCLUDE_DIRS})
target_include_directories(gl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(gl PRIVATE /W3)
else()
    target_compile_options(gl PRIVATE -Wall -Wextra -Wpedantic)
endif()