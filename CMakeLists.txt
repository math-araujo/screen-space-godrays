cmake_minimum_required(VERSION 3.25)
include(cmake/fetchdependencies.cmake)
project(screen-space-godrays LANGUAGES CXX)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")

add_subdirectory(src)