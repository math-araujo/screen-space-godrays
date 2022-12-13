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
    default_cube_ = std::make_unique<gl::Mesh>(
        // clang-format off
        std::vector<float>{
        // Back face
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, // Bottom-left
         1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, // top-right
         1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, // bottom-right         
         1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, // top-right
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, // bottom-left
        -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, // top-left
        // Front face
        -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // bottom-left
         1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        // Left face
        -1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, // top-right
        // Right face
         1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // top-right         
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, // bottom-left     
        // Bottom face
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, // top-right
         1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, // top-right
        // Top face
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // top-left
         1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // top-right     
         1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // top-left
        -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f  // bottom-left        
        },
        std::vector<int>{3, 3});
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
    shader_->set_vec3_uniform("light.direction", light_.direction);
    shader_->set_vec3_uniform("light.ambient", light_.ambient);
    shader_->set_vec3_uniform("light.diffuse", light_.diffuse);
    shader_->set_vec3_uniform("light.specular", light_.specular);
    shader_->set_vec3_uniform("view_pos", camera().position());
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

    ImGui::Begin("Settings");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    if (ImGui::TreeNode("Light"))
    {
        ImGui::SliderFloat3("Direction", glm::value_ptr(light_.direction), -20.0f, 20.0f);
        // ImGui::SliderFloat3("Diffuse", glm::value_ptr(light_.diffuse), 0.0f, 1.0f);

        ImGui::TreePop();
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}