#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// GLAD must be imported before GLFW
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "main_application.hpp"

MainApplication::MainApplication(int window_width, int window_height, std::string_view title) :
    gl::Application(window_width, window_height, title)
{
    shader_ = std::make_unique<gl::ShaderProgram>(std::initializer_list<std::pair<std::string_view, gl::Shader::Type>>{
        {"assets/shaders/phong/vertex.glsl", gl::Shader::Type::Vertex},
        {"assets/shaders/phong/fragment.glsl", gl::Shader::Type::Fragment}});
    default_cube_ = std::make_unique<gl::IndexedMesh>(
        // clang-format off
        std::vector<float>{
             1.0,  1.0, -1.0,
             1.0, -1.0, -1.0,
             1.0,  1.0,  1.0,
             1.0, -1.0,  1.0,
            -1.0,  1.0, -1.0,
            -1.0, -1.0, -1.0,
            -1.0,  1.0,  1.0,
            -1.0, -1.0,  1.0
        },
        std::vector<std::uint32_t>{
            4, 2, 0,
            2, 7, 3,
            6, 5, 7,
            1, 7, 5,
            0, 3, 1,
            4, 1, 5,
            4, 6, 2,
            2, 6, 7,
            6, 4, 5,
            1, 3, 7,
            0, 2, 3,
            4, 0, 1
        },
        std::vector<int>{3});
    // clang-format on
}

void MainApplication::render()
{
    glGetIntegerv(GL_VIEWPORT, current_viewport_.data());

    // Clear window with specified color
    glClearColor(0.1f, 0.1f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene...
    shader_->use();
    shader_->set_mat4_uniform("mvp", camera().view_projection());
    default_cube_->render();

    // Render GUI
    render_imgui_editor();
}

void MainApplication::render_imgui_editor()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("FPS");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}