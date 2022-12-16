#ifndef MAIN_APPLICATION_HPP
#define MAIN_APPLICATION_HPP

#include <string_view>

#include "gl/application.hpp"
#include "gl/framebuffer.hpp"
#include "gl/light.hpp"
#include "gl/model.hpp"
#include "gl/shader.hpp"

class MainApplication : public gl::Application
{
public:
    MainApplication(int window_width, int window_height, std::string_view title);
    MainApplication(const MainApplication&) = delete;
    MainApplication(MainApplication&&) = delete;
    MainApplication& operator=(const MainApplication&) = delete;
    MainApplication& operator=(MainApplication&&) = delete;
    ~MainApplication() override = default;

    void render() override;
    void render_imgui_editor() override;

private:
    enum class RenderMode
    {
        CompleteRender = 0,
        DefaultSceneOnly,
        OcclusionMapOnly,
        RadialBlurOnly
    };

    std::unique_ptr<gl::ShaderProgram> blinn_phong_shader_{};
    std::unique_ptr<gl::ShaderProgram> basic_shader_{};
    std::unique_ptr<gl::ShaderProgram> post_process_shader_{};
    std::unique_ptr<gl::Framebuffer> occlusion_fbo_{};
    std::unique_ptr<gl::IndexedMesh> full_screen_quad_{};
    std::unordered_map<std::string, gl::Model> models_{};
    RenderMode render_mode_{RenderMode::CompleteRender};
    gl::DirectionalLight light_{.direction = glm::vec3{1.0f, 1.0f, 1.0f},
                                .ambient = glm::vec3{0.2f, 0.0f, 0.2f},
                                .diffuse = glm::vec3{0.6f, 0.0f, 0.6f},
                                .specular = glm::vec3{0.2f, 0.0f, 0.2f}};
};

#endif // MAIN_APPLICATION_HPP