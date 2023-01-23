#include "application.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// GLAD must be imported before GLFW
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <exception>
#include <iostream>
#include <string>

#include "framebuffer.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "skybox.hpp"
#include "texture.hpp"

namespace gl
{

Application::Application(int window_width, int window_height, std::string_view title) :
    width_{window_width}, height_{window_height}, aspect_ratio_{static_cast<float>(width_) / height_}
{
    create_context(title);
    initialize_imgui();
    load_opengl();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
}

void Application::create_context(std::string_view title)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        glfwTerminate();
        throw std::runtime_error("Failure to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    window_ = glfwCreateWindow(width_, height_, title.data(), nullptr, nullptr);
    if (!window_)
    {
        glfwTerminate();
        throw std::runtime_error("Failure to create OpenGL context or GLFW window");
    }

    glfwMakeContextCurrent(window_);
    glfwSetWindowSizeCallback(window_, framebuffer_size_callback);
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, key_callback);
    glfwSetCursorPosCallback(window_, mouse_movement_callback);
    glfwSetMouseButtonCallback(window_, mouse_button_callback);
    glfwSetScrollCallback(window_, scroll_callback);
}

void error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
    Application* application = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        if (application->is_wireframe_mode())
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        application->switch_wireframe_mode();
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        if (application->is_mouse_movement_free())
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        application->switch_free_mouse_movement();
    }
}

bool Application::is_wireframe_mode() const
{
    return wireframe_mode_;
}

void Application::switch_wireframe_mode()
{
    wireframe_mode_ = !wireframe_mode_;
}

bool Application::is_mouse_movement_free() const
{
    return free_mouse_move_;
}

void Application::switch_free_mouse_movement()
{
    free_mouse_move_ = !free_mouse_move_;
}

void mouse_movement_callback(GLFWwindow* window, double x_pos, double y_pos)
{
    Application* application = static_cast<Application*>(glfwGetWindowUserPointer(window));
    static bool first_mouse{true};
    static glm::vec2 last_position{0.0f, 0.0f};

    if (first_mouse)
    {
        last_position = {static_cast<float>(x_pos), static_cast<float>(y_pos)};
        first_mouse = false;
    }

    const float x_offset{static_cast<float>(x_pos - last_position.x)};
    const float y_offset{static_cast<float>(last_position.y - y_pos)};
    last_position = {static_cast<float>(x_pos), static_cast<float>(y_pos)};
    if (application->is_mouse_movement_free() || application->mouse_clicking())
    {
        application->camera().process_mouse_movement(x_offset, y_offset);
    }
}

bool Application::mouse_clicking() const
{
    return mouse_click_;
}

FPSCamera& Application::camera()
{
    return camera_;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/)
{
    Application* application = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        switch (action)
        {
        case GLFW_PRESS:
            application->set_mouse_click(true);
            break;
        case GLFW_RELEASE:
            application->set_mouse_click(false);
            break;
        default:
            break;
        }
    }
}

void Application::set_mouse_click(bool mouse_click)
{
    mouse_click_ = mouse_click;
}

void scroll_callback(GLFWwindow* window, double /*x_offset*/, double y_offset)
{
    Application* application = static_cast<Application*>(glfwGetWindowUserPointer(window));
    application->camera().process_mouse_scroll(y_offset);
}

void Application::initialize_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io; // Use io in a statement to avoid unused variable warning
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("# version 450");
}

void Application::load_opengl()
{
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
        throw std::runtime_error("Failure to initialize GLAD");
    }
}

Application::~Application()
{
    cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void Application::cleanup()
{
}

void Application::run()
{
    float delta_time = 0.0f;
    float previous_time = 0.0f;

    while (!glfwWindowShouldClose(window_))
    {
        float current_time = static_cast<float>(glfwGetTime());
        delta_time = current_time - previous_time;
        previous_time = current_time;

        process_input(delta_time);
        update(delta_time);
        render();
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

void Application::process_input(float delta_time)
{
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window_, true);
        return;
    }

    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera_.process_keyboard_input(CameraMovement::Forward, delta_time);
    }

    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera_.process_keyboard_input(CameraMovement::Backward, delta_time);
    }

    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera_.process_keyboard_input(CameraMovement::Right, delta_time);
    }

    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera_.process_keyboard_input(CameraMovement::Left, delta_time);
    }

    if (glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera_.process_keyboard_input(CameraMovement::Up, delta_time);
    }

    if (glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera_.process_keyboard_input(CameraMovement::Down, delta_time);
    }
}

void Application::update(float /*delta_time*/)
{
}

void Application::render()
{
    glGetIntegerv(GL_VIEWPORT, current_viewport_.data());

    // Clear window with specified color
    glClearColor(0.0f, 0.1f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene...

    // Render GUI
    render_imgui_editor();
}

void Application::reset_viewport()
{
    glViewport(current_viewport_[0], current_viewport_[1], current_viewport_[2], current_viewport_[3]);
}

void Application::render_imgui_editor()
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

} // namespace gl