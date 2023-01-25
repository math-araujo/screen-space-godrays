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
        DefaultSceneOnly = 0,
        OcclusionMapOnly,
        RadialBlurOnly,
        CompleteRender
    };

    struct PostprocessingCoefficients
    {
        int num_samples;
        float density;
        float exposure;
        float decay;
        float weight;
    };

    struct ShadowMapParameters
    {
        float near_plane{0.1f};
        float far_plane{100.0f};
        float frustum_dimension{24.2f};
        float bias{0.005f};
        glm::mat4 light_projection;
        glm::vec3 target{0.0f};

        void set_projection();
    };

    std::unique_ptr<gl::ShaderProgram> texture_blinn_phong_shader_{};
    std::unique_ptr<gl::ShaderProgram> color_blinn_phong_shader_{};
    std::unique_ptr<gl::ShaderProgram> color_shader_{};
    std::unique_ptr<gl::ShaderProgram> post_process_shader_{};
    std::unique_ptr<gl::ShaderProgram> shadow_map_shader_{};
    std::unique_ptr<gl::Framebuffer> occlusion_fbo_{};
    std::unique_ptr<gl::Framebuffer> shadow_map_fbo_{};
    std::unique_ptr<gl::IndexedMesh> full_screen_quad_{};
    std::unordered_map<std::string, gl::Model> models_{};
    RenderMode render_mode_{RenderMode::CompleteRender};
    gl::DirectionalLight light_{.direction = glm::vec3{1.0f, 1.0f, 1.0f},
                                .ambient = glm::vec3{0.2f, 0.2f, 0.2f},
                                .diffuse = glm::vec3{0.6f, 0.6f, 0.6f},
                                .specular = glm::vec3{0.2f, 0.2f, 0.2f}};
    PostprocessingCoefficients coefficients{
        .num_samples = 100, .density = 1.0f, .exposure = 1.0f, .decay = 1.0f, .weight = 0.01f};
    bool apply_radial_blur_{true};
    ShadowMapParameters shadow_map_parameters_{};

    void set_shadow_map_transforms();
};

#endif // MAIN_APPLICATION_HPP