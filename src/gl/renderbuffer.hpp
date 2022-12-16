#ifndef RENDERBUFFER_HPP
#define RENDERBUFFER_HPP

#include <cstdint>

#include <glad/glad.h>

namespace gl
{

class Renderbuffer
{
public:
    Renderbuffer(std::uint32_t width, std::uint32_t height, GLenum internal_format);
    Renderbuffer(const Renderbuffer&) = delete;
    Renderbuffer(Renderbuffer&& other) noexcept;
    Renderbuffer& operator=(const Renderbuffer&) = delete;
    Renderbuffer& operator=(Renderbuffer&&) noexcept;
    ~Renderbuffer();

    std::uint32_t id() const;
    std::uint32_t width() const;
    std::uint32_t height() const;

private:
    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t id_{0};
};

} // namespace gl

#endif // RENDERBUFFER_HPP