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
#include <optional>
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

    shadow_map_shader_ = std::make_unique<gl::ShaderProgram>(
        std::initializer_list<gl::ShaderInfo>{{"assets/shaders/shadow_map/vertex.glsl", gl::Shader::Type::Vertex},
                                              {"assets/shaders/shadow_map/fragment.glsl", gl::Shader::Type::Fragment}});
    // Create framebuffer objects
    const std::uint32_t half_width{static_cast<std::uint32_t>(window_width / 2)};
    const std::uint32_t half_height{static_cast<std::uint32_t>(window_height / 2)};
    occlusion_fbo_ = std::make_unique<gl::Framebuffer>(half_width, half_height,
                                                       gl::Renderbuffer{half_width, half_height, GL_DEPTH_COMPONENT32},
                                                       gl::Texture{half_width, half_height});

    gl::Texture shadow_depth_map{1024, 1024,
                                 gl::Texture::Attributes{.wrap_s = GL_CLAMP_TO_BORDER,
                                                         .wrap_t = GL_CLAMP_TO_BORDER,
                                                         .min_filter = GL_NEAREST,
                                                         .mag_filter = GL_NEAREST,
                                                         .internal_format = GL_DEPTH_COMPONENT32F,
                                                         .pixel_data_format = GL_DEPTH_COMPONENT,
                                                         .pixel_data_type = GL_FLOAT}};
    shadow_map_fbo_ = std::make_unique<gl::Framebuffer>(1024, 1024, std::move(shadow_depth_map), std::nullopt);
    shadow_map_fbo_->set_depth_border({1.0f, 1.0f, 1.0f, 1.0f});

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
    models_.merge(gl::read_triangle_mesh("sibenik.obj"));
    models_.at("sibenik").sort_by_texture();
    light_.direction = glm::vec3{17.143f, 6.857f, 4.225f};
    models_.at("UVSphere").translation = light_.direction;
    models_.at("arclight").scale = glm::vec3{1.6f, 2.0f, 1.5f};
    models_.at("arclight").translation = glm::vec3{18.5f, 10.0f, 6.0f};

    // Set light uniforms
    texture_blinn_phong_shader_->set_vec3_uniform("light.direction", light_.direction);
    texture_blinn_phong_shader_->set_vec3_uniform("light.ambient", light_.ambient);
    texture_blinn_phong_shader_->set_vec3_uniform("light.diffuse", light_.diffuse);
    texture_blinn_phong_shader_->set_vec3_uniform("light.specular", light_.specular);
    texture_blinn_phong_shader_->set_float_uniform("bias", shadow_map_parameters_.bias);
    color_blinn_phong_shader_->set_vec3_uniform("light.direction", light_.direction);
    color_blinn_phong_shader_->set_vec3_uniform("light.ambient", light_.ambient);
    color_blinn_phong_shader_->set_vec3_uniform("light.diffuse", light_.diffuse);
    color_blinn_phong_shader_->set_vec3_uniform("light.specular", light_.specular);
    color_blinn_phong_shader_->set_float_uniform("bias", shadow_map_parameters_.bias);

    post_process_shader_->set_bool_uniform("apply_radial_blur", apply_radial_blur_);
    post_process_shader_->set_int_uniform("coefficients.num_samples", coefficients.num_samples);
    post_process_shader_->set_float_uniform("coefficients.density", coefficients.density);
    post_process_shader_->set_float_uniform("coefficients.exposure", coefficients.exposure);
    post_process_shader_->set_float_uniform("coefficients.decay", coefficients.decay);
    post_process_shader_->set_float_uniform("coefficients.weight", coefficients.weight);

    shadow_map_parameters_.set_projection();
}

void MainApplication::render()
{
    glGetIntegerv(GL_VIEWPORT, current_viewport_.data());
    const glm::mat4& view_projection{camera().view_projection()};
    auto& sibenik = models_.at("sibenik");
    auto& arclight = models_.at("arclight");
    auto& uv_sphere = models_.at("UVSphere");

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
    color_shader_->set_mat4_uniform("mvp", view_projection * sibenik.transform());
    sibenik.render_opaque_meshes();
    color_shader_->set_vec4_uniform("color", glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
    std::vector<gl::Model*> light_models{&arclight};
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
    // Shadow map render pass
    const glm::mat4 light_view{
        glm::lookAt(uv_sphere.translation, shadow_map_parameters_.target, glm::vec3{0.0f, 1.0f, 0.0f})};
    const glm::mat4 light_space_transform{shadow_map_parameters_.light_projection * light_view};
    shadow_map_fbo_->bind();
    shadow_map_shader_->use();
    shadow_map_shader_->set_mat4_uniform("light_space_transform", light_space_transform);
    shadow_map_shader_->set_mat4_uniform("model", sibenik.transform());
    sibenik.render_opaque_meshes();
    shadow_map_fbo_->unbind();
    reset_viewport();

    // First render opaque objects
    texture_blinn_phong_shader_->use();
    // TODO: refactor "view_pos", "mvp", "model" and "light_space_transform" as UBOs to avoid sending the same data to
    // the GPU
    texture_blinn_phong_shader_->set_vec3_uniform("view_pos", camera().position());
    texture_blinn_phong_shader_->set_mat4_uniform("mvp", view_projection * sibenik.transform());
    texture_blinn_phong_shader_->set_mat4_uniform("model", sibenik.transform());
    texture_blinn_phong_shader_->set_mat4_uniform("light_space_transform", light_space_transform);
    shadow_map_fbo_->bind_depth_texture(1);
    sibenik.render_textured_meshes();
    color_blinn_phong_shader_->use();
    color_blinn_phong_shader_->set_vec3_uniform("view_pos", camera().position());
    color_blinn_phong_shader_->set_mat4_uniform("mvp", view_projection * sibenik.transform());
    color_blinn_phong_shader_->set_mat4_uniform("model", sibenik.transform());
    color_blinn_phong_shader_->set_mat4_uniform("light_space_transform", light_space_transform);
    sibenik.render_colored_meshes(*color_blinn_phong_shader_, "diffuse_color");
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
    color_shader_->set_mat4_uniform("mvp", view_projection * sibenik.transform());
    sibenik.render_semitransparent_meshes(*color_shader_, "color");

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

void MainApplication::ShadowMapParameters::set_projection()
{
    light_projection =
        glm::ortho(-frustum_dimension, frustum_dimension, -frustum_dimension, frustum_dimension, near_plane, far_plane);
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
        ImGui::RadioButton("Default Scene Only", &render_mode_value, static_cast<int>(RenderMode::DefaultSceneOnly));
        ImGui::RadioButton("Occlusion Map Only", &render_mode_value, static_cast<int>(RenderMode::OcclusionMapOnly));
        ImGui::RadioButton("Radial Blur Only", &render_mode_value, static_cast<int>(RenderMode::RadialBlurOnly));
        ImGui::RadioButton("Complete Scene", &render_mode_value, static_cast<int>(RenderMode::CompleteRender));
        render_mode_ = static_cast<RenderMode>(render_mode_value);
        ImGui::TreePop();
    }

    // See Mitchell "Volumetric Light Scattering as a Post-Process" for a detailed explanation
    // of each post-processing parameter.
    if (ImGui::TreeNode("Post-processing Coefficients"))
    {
        if (ImGui::InputInt("Number of samples", &coefficients.num_samples))
        {
            coefficients.num_samples = std::clamp(coefficients.num_samples, 0, 512);
            post_process_shader_->set_int_uniform("coefficients.num_samples", coefficients.num_samples);
        }

        if (ImGui::SliderFloat("Density", &coefficients.density, 0.0f, 2.0f))
        {
            post_process_shader_->set_float_uniform("coefficients.density", coefficients.density);
        }

        if (ImGui::SliderFloat("Exposure", &coefficients.exposure, 0.0f, 10.0f))
        {
            post_process_shader_->set_float_uniform("coefficients.exposure", coefficients.exposure);
        }

        if (ImGui::SliderFloat("Decay", &coefficients.decay, 0.0f, 1.0f))
        {
            post_process_shader_->set_float_uniform("coefficients.decay", coefficients.decay);
        }

        if (ImGui::SliderFloat("Weight", &coefficients.weight, 0.0f, 0.1f))
        {
            post_process_shader_->set_float_uniform("coefficients.weight", coefficients.weight);
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Shadow Mapping Parameters"))
    {
        if (ImGui::SliderFloat("Near plane", &shadow_map_parameters_.near_plane, 0.0f, 2.0f))
        {
            shadow_map_parameters_.set_projection();
        }

        if (ImGui::SliderFloat("Far plane", &shadow_map_parameters_.far_plane, 2.0f, 200.0f))
        {
            shadow_map_parameters_.set_projection();
        }

        if (ImGui::SliderFloat("Frustum Dimensions", &shadow_map_parameters_.frustum_dimension, 1.0f, 50.0f))
        {
            shadow_map_parameters_.set_projection();
        }

        if (ImGui::SliderFloat("Shadow Bias", &shadow_map_parameters_.bias, 0.001f, 0.01f))
        {
            texture_blinn_phong_shader_->set_float_uniform("bias", shadow_map_parameters_.bias);
            color_blinn_phong_shader_->set_float_uniform("bias", shadow_map_parameters_.bias);
        }

        ImGui::SliderFloat3("Target position (lookAt)", glm::value_ptr(shadow_map_parameters_.target), -10.0f, 10.0f);

        ImGui::TreePop();
    }

    if (ImGui::SliderFloat3("Light Direction", glm::value_ptr(light_.direction), -20.0f, 20.0f))
    {
        models_.at("UVSphere").translation = light_.direction;
        texture_blinn_phong_shader_->set_vec3_uniform("light.direction", light_.direction);
        color_blinn_phong_shader_->set_vec3_uniform("light.direction", light_.direction);
    }

    ImTextureID imgui_texture_id = reinterpret_cast<void*>(static_cast<std::intptr_t>(shadow_map_fbo_->depth_id()));
    ImGui::Image(imgui_texture_id, ImVec2{200, 200}, ImVec2{0.0f, 0.0f}, ImVec2{1.0f, 1.0f},
                 ImVec4{1.0f, 1.0f, 1.0f, 1.0f}, ImVec4{1.0f, 1.0f, 1.0f, 0.5f});

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}