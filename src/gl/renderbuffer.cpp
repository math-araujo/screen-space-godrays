#include "renderbuffer.hpp"

#include <algorithm>

namespace gl
{

Renderbuffer::Renderbuffer(std::uint32_t width, std::uint32_t height, GLenum internal_format) :
    width_{width}, height_{height}
{
    glCreateRenderbuffers(1, &id_);
    glNamedRenderbufferStorage(id_, internal_format, width_, height_);
}

Renderbuffer::Renderbuffer(Renderbuffer&& other) noexcept : width_{other.width_}, height_{other.height_}, id_{other.id_}
{
    other.id_ = 0;
}

Renderbuffer& Renderbuffer::operator=(Renderbuffer&& other) noexcept
{
    std::swap(width_, other.width_);
    std::swap(height_, other.height_);
    std::swap(id_, other.id_);
    return *this;
}

Renderbuffer::~Renderbuffer()
{
    glDeleteRenderbuffers(1, &id_);
}

std::uint32_t Renderbuffer::id() const
{
    return id_;
}

std::uint32_t Renderbuffer::width() const
{
    return width_;
}

std::uint32_t Renderbuffer::height() const
{
    return height_;
}

} // namespace gl