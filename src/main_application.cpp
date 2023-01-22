#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// GLAD must be imported before GLFW
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vector>

#include "gl/io.hpp"
#include "gl/texture.hpp"
#include "main_application.hpp"

MainApplication::MainApplication(int window_width, int window_height, std::string_view title) :
    gl::Application(window_width, window_height, title)
{
    camera().set_position(glm::vec3{-5.73f, 1.91f, 2.15f});
    camera().set_pitch_yaw(glm::vec2{3.0f, 84.5});

    // Create shaders
    texture_blinn_phong_shader_ = std::make_unique<gl::ShaderProgram>(std::initializer_list<gl::ShaderInfo>{
        {"assets/shaders/phong/vertex.glsl", gl::Shader::Type::Vertex},
        {"assets/shaders/phong/fragment.glsl", gl::Shader::Type::Fragment, {"DIFFUSE_MAP"}}});

    color_blinn_phong_shader_ = std::make_unique<gl::ShaderProgram>(
        std::initializer_list<gl::ShaderInfo>{{"assets/shaders/phong/vertex.glsl", gl::Shader::Type::Vertex},
                                              {"assets/shaders/phong/fragment.glsl", gl::Shader::Type::Fragment}});

    color_shader_ = std::make_unique<gl::ShaderProgram>(
        std::initializer_list<gl::ShaderInfo>{{"assets/shaders/basic/vertex.glsl", gl::Shader::Type::Vertex},
                                              {"assets/shaders/basic/fragment.glsl", gl::Shader::Type::Fragment}});

    post_process_shader_ = std::make_unique<gl::ShaderProgram>(std::initializer_list<gl::ShaderInfo>{
        {"assets/shaders/post_process/vertex.glsl", gl::Shader::Type::Vertex},
        {"assets/shaders/post_process/fragment.glsl", gl::Shader::Type::Fragment, {"NUM_LIGHTS 1"}}});

    // Create framebuffer objects
    const std::uint32_t half_width{static_cast<std::uint32_t>(window_width / 2)};
    const std::uint32_t half_height{static_cast<std::uint32_t>(window_height / 2)};
    occlusion_fbo_ = std::make_unique<gl::Framebuffer>(half_width, half_height,
                                                       gl::Renderbuffer{half_width, half_height, GL_DEPTH_COMPONENT32},
                                                       gl::Texture{half_width, half_height});

    // clang-format off
    full_screen_quad_ = std::make_unique<gl::IndexedMesh>(
        std::vector<float>{
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // Top-Right
            -1.0f,  1.0f, 0.0,  0.0f, 1.0f, // Top-Left
            -1.0f, -1.0f, 0.0,  0.0f, 0.0f, // Bottom-Left
             1.0f, -1.0f, 0.0,  1.0f, 0.0f  // Bottom-Right 
        },
        std::vector<std::uint32_t>{0, 1, 3, 3, 1, 2},
        std::vector<int>{3, 2});
    // clang-format on

    // Read and initialize models
    models_ = gl::read_triangle_mesh("uv_sphere.obj");
    models_.merge(gl::read_triangle_mesh("arclight.obj"));
    models_.merge(gl::read_triangle_mesh("sibenik.obj", true));
    models_.at("sibenik").sort_by_texture();
    models_.at("UVSphere").translation = glm::vec3{0.0f, 5.0f, -50.0f};
    models_.at("arclight").scale = glm::vec3{1.6f, 2.0f, 1.5f};
    models_.at("arclight").translation = glm::vec3{18.5f, 10.0f, 6.0f};

    // Set light uniforms
    texture_blinn_phong_shader_->set_vec3_uniform("light.direction", light_.direction);
    texture_blinn_phong_shader_->set_vec3_uniform("light.ambient", light_.ambient);
    texture_blinn_phong_shader_->set_vec3_uniform("light.diffuse", light_.diffuse);
    texture_blinn_phong_shader_->set_vec3_uniform("light.specular", light_.specular);
    color_blinn_phong_shader_->set_vec3_uniform("light.direction", light_.direction);
    color_blinn_phong_shader_->set_vec3_uniform("light.ambient", light_.ambient);
    color_blinn_phong_shader_->set_vec3_uniform("light.diffuse", light_.diffuse);
    color_blinn_phong_shader_->set_vec3_uniform("light.specular", light_.specular);

    post_process_shader_->set_bool_uniform("apply_radial_blur", apply_radial_blur_);
    post_process_shader_->set_int_uniform("coefficients.num_samples", coefficients.num_samples);
    post_process_shader_->set_float_uniform("coefficients.density", coefficients.density);
    post_process_shader_->set_float_uniform("coefficients.exposure", coefficients.exposure);
    post_process_shader_->set_float_uniform("coefficients.decay", coefficients.decay);
    post_process_shader_->set_float_uniform("coefficients.weight", coefficients.weight);
}

void MainApplication::render()
{
    glGetIntegerv(GL_VIEWPORT, current_viewport_.data());
    const glm::mat4& view_projection{camera().view_projection()};
    /*
    Occlusion Pre-Pass Method:
    Render the scene geometry as black and light source with the
    desired color. The color buffer of the occlusion framebuffer
    stores the occlusion map, which will be used in the
    post-processing phase to gneerate the god rays.
    */
    occlusion_fbo_->bind();
    color_shader_->use();
    color_shader_->set_vec4_uniform("color", glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
    color_shader_->set_mat4_uniform("mvp", view_projection * models_.at("sibenik").transform());
    models_.at("sibenik").render_opaque_meshes();
    color_shader_->set_vec4_uniform("color", glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
    std::vector<gl::Model*> light_models{&models_.at("arclight")};
    for (auto& light : light_models)
    {
        color_shader_->set_mat4_uniform("mvp", view_projection * light->transform());
        light->render();
    }
    occlusion_fbo_->unbind();

    reset_viewport();

    // Clear window with specified color
    glClearColor(0.05f, 0.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Second Render Pass: render scene as usual
    // First render opaque objects
    texture_blinn_phong_shader_->use();
    // TODO: maybe refactor "view_pos", "mvp" and "model" as UBOs to avoid sending the same data to the GPU
    texture_blinn_phong_shader_->set_vec3_uniform("view_pos", camera().position());
    texture_blinn_phong_shader_->set_mat4_uniform("mvp", view_projection * models_.at("sibenik").transform());
    texture_blinn_phong_shader_->set_mat4_uniform("model", models_.at("sibenik").transform());
    models_.at("sibenik").render_textured_meshes();
    color_blinn_phong_shader_->use();
    color_blinn_phong_shader_->set_vec3_uniform("view_pos", camera().position());
    color_blinn_phong_shader_->set_mat4_uniform("mvp", view_projection * models_.at("sibenik").transform());
    color_blinn_phong_shader_->set_mat4_uniform("model", models_.at("sibenik").transform());
    models_.at("sibenik").render_colored_meshes(*color_blinn_phong_shader_, "diffuse_color");
    color_shader_->use();
    color_shader_->set_vec4_uniform("color", glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
    for (auto& light : light_models)
    {
        color_shader_->set_mat4_uniform("mvp", view_projection * light->transform());
        light->render();
    }
    // Render (semi)transparent objects after opaque objects
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    color_shader_->set_mat4_uniform("mvp", view_projection * models_.at("sibenik").transform());
    models_.at("sibenik").render_semitransparent_meshes(*color_shader_, "color");

    /*
    Post-Processing God Rays Render Pass:
    without clearing the default framebuffer, bind the occlusion map
    and apply a radial blur to it. Finally, a blending function is
    applied between the default render (renderer on the second pass)
    and the radial blur.
    */

    switch (render_mode_)
    {
    case RenderMode::DefaultSceneOnly:
        glBlendFunc(GL_ZERO, GL_ONE);
        break;
    case RenderMode::OcclusionMapOnly:
        glBlendFunc(GL_ONE, GL_ZERO);
        apply_radial_blur_ = false;
        break;
    case RenderMode::RadialBlurOnly:
        glBlendFunc(GL_ONE, GL_ZERO);
        apply_radial_blur_ = true;
        break;
    case RenderMode::CompleteRender:
    default:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        apply_radial_blur_ = true;
        break;
    }

    post_process_shader_->use();
    post_process_shader_->set_bool_uniform("apply_radial_blur", apply_radial_blur_);
    occlusion_fbo_->bind_color(0);

    // Each light source is initially centered at origin; the model matrix is responsible
    // for updating it's position in the world space.
    std::vector<glm::vec4> light_positions;
    for (auto& light : light_models)
    {
        const glm::vec4 clip_light_position{view_projection * light->transform() * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
        const glm::vec4 ndc_light_position{clip_light_position / clip_light_position.w};
        const glm::vec4 screen_space_light_position{(ndc_light_position + 1.0f) * 0.5f};
        light_positions.emplace_back(screen_space_light_position);
    }
    post_process_shader_->set_vec4_array_uniform("screen_space_light_positions[0]", light_positions);
    full_screen_quad_->render();

    glDisable(GL_BLEND);

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

    int render_mode_value{static_cast<int>(render_mode_)};
    if (ImGui::TreeNode("Render Mode"))
    {
        ImGui::TreePop();
        ImGui::RadioButton("Default Scene Only", &render_mode_value, static_cast<int>(RenderMode::DefaultSceneOnly));
        ImGui::RadioButton("Occlusion Map Only", &render_mode_value, static_cast<int>(RenderMode::OcclusionMapOnly));
        ImGui::RadioButton("Radial Blur Only", &render_mode_value, static_cast<int>(RenderMode::RadialBlurOnly));
        ImGui::RadioButton("Complete Scene", &render_mode_value, static_cast<int>(RenderMode::CompleteRender));
        render_mode_ = static_cast<RenderMode>(render_mode_value);
    }
    if (ImGui::InputFloat3("Light Dir", glm::value_ptr(models_.at("UVSphere").translation)))
    {
        light_.direction = glm::normalize(-models_.at("UVSphere").translation);
        texture_blinn_phong_shader_->set_vec3_uniform("light.direction", light_.direction);
        color_blinn_phong_shader_->set_vec3_uniform("light.direction", light_.direction);
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}