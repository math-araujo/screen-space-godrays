#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <array>
#include <memory>
#include <string_view>

#include <glm/glm.hpp>

#include "camera.hpp"

struct GLFWwindow;

namespace gl
{

class Application
{
public:
    Application(int window_width, int window_height, std::string_view title);
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;
    virtual ~Application();

    virtual void run();
    virtual void process_input(float delta_time);
    virtual void update(float delta_time);
    virtual void render();
    virtual void render_imgui_editor();

    // Functions to interact with GLFW callback functions
    FPSCamera& camera();
    bool is_wireframe_mode() const;
    void switch_wireframe_mode();
    void set_mouse_click(bool mouse_click);
    bool mouse_clicking() const;
    void switch_free_mouse_movement();
    bool is_mouse_movement_free() const;

    /*
    Reset viewport to the Application's width and height values
    */
    void reset_viewport();

protected:
    const int width_;
    const int height_;
    const float aspect_ratio_;

    std::array<int, 4> current_viewport_{};
    GLFWwindow* window_{nullptr};
    bool wireframe_mode_{false};
    bool mouse_click_{false};
    bool free_mouse_move_{false};

    FPSCamera camera_{glm::vec3{0.0, 0.0f, 3.0f}};

private:
    /*
    Create a window and OpenGL context. If creation
    was unsuccesfull, throws a runtime exception.
    */
    void create_context(std::string_view title);

    /*
    Initializes ImGui
    */
    void initialize_imgui();

    /*
    Load OpenGL functions. If loading was unsuccessfull,
    throws a runtime exception.
    */
    void load_opengl();

    /*
    Manually cleanup OpenGL-related objects. Since Application
    destructor terminates the OpenGL context, it's necessary
    to manually cleanup all OpenGL-related objects prior to
    the termination of the context.
    */
    void cleanup();
};

void error_callback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_movement_callback(GLFWwindow* window, double x_pos, double y_pos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);

} // namespace gl

#endif // APPLICATION_HPP