#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// GLAD must be imported before GLFW
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "gl/io.hpp"
#include "main_application.hpp"

MainApplication::MainApplication(int window_width, int window_height, std::string_view title) :
    gl::Application(window_width, window_height, title)
{
    camera().set_position(glm::vec3{0.0f, 0.0f, 20.0f});
    shader_ = std::make_unique<gl::ShaderProgram>(
        std::initializer_list<gl::ShaderInfo>{{"assets/shaders/phong/vertex.glsl", gl::Shader::Type::Vertex},
                                              {"assets/shaders/phong/fragment.glsl", gl::Shader::Type::Fragment}});
    basic_shader_ = std::make_unique<gl::ShaderProgram>(
        std::initializer_list<gl::ShaderInfo>{{"assets/shaders/basic/vertex.glsl", gl::Shader::Type::Vertex},
                                              {"assets/shaders/basic/fragment.glsl", gl::Shader::Type::Fragment}});
    auto meshes = gl::read_triangle_mesh("assets/models/uv_sphere.obj");
    meshes.merge(gl::read_triangle_mesh("assets/models/cube.obj"));
    for (auto& pair : meshes)
    {
        gl::Model model{.mesh = std::move(pair.second)};
        models_.emplace(pair.first, std::move(model));
    }
    models_.at("UVSphere").translation = glm::vec3{0.0f, 5.0f, -50.0f};
    light_.direction = glm::normalize(-models_.at("UVSphere").translation);
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
    shader_->set_mat4_uniform("mvp", camera().view_projection() * models_.at("Cube").transform());
    models_.at("Cube").mesh.render();
    basic_shader_->use();
    basic_shader_->set_mat4_uniform("mvp", camera().view_projection() * models_.at("UVSphere").transform());
    models_.at("UVSphere").mesh.render();

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
    /*if (ImGui::TreeNode("Light"))
    {
        ImGui::SliderFloat3("Direction", glm::value_ptr(light_.direction), -20.0f, 20.0f);

        ImGui::TreePop();
    }*/
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}