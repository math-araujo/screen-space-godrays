#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <array>
#include <cstdint>
#include <optional>
#include <variant>

#include <glad/glad.h>

#include "renderbuffer.hpp"
#include "texture.hpp"

namespace gl
{

class Framebuffer
{
public:
    Framebuffer(std::uint32_t width, std::uint32_t height, Texture depth, std::optional<Texture> color);
    Framebuffer(std::uint32_t width, std::uint32_t height, Renderbuffer depth, std::optional<Texture> color);
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer& operator=(Framebuffer&& other) noexcept;
    ~Framebuffer();

    void clear();
    void bind();
    void unbind();

    std::uint32_t width() const;
    std::uint32_t height() const;
    std::uint32_t id() const;
    std::uint32_t color_id() const;
    std::uint32_t depth_id() const;
    void bind_color(std::uint32_t texture_unit);
    void bind_depth_texture(std::uint32_t texture_unit);
    void set_depth_border(const std::array<float, 4>& border);
    void set_color_border(const std::array<float, 4>& border);

private:
    inline static const std::array<float, 4> clear_color_{0.0f, 0.0f, 0.0f, 1.0f};
    inline static const float clear_depth_{1.0f};

    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t id_{0};
    std::variant<Texture, Renderbuffer> depth_;
    std::optional<Texture> color_;

    void initialize(bool use_depth_renderbuffer);
};

} // namespace gl

#endif // FRAMEBUFFER_HPP