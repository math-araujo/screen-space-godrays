#ifndef MAIN_APPLICATION_HPP
#define MAIN_APPLICATION_HPP

#include <string_view>

#include "gl/application.hpp"
#include "gl/mesh.hpp"
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
    std::unique_ptr<gl::ShaderProgram> shader_{};
    std::unique_ptr<gl::IndexedMesh> default_cube_{};
};

#endif // MAIN_APPLICATION_HPP