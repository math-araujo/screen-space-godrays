#include "framebuffer.hpp"

#include <cassert>
#include <exception>
#include <iostream>

namespace gl
{

Framebuffer::Framebuffer(std::uint32_t width, std::uint32_t height, Texture depth, std::optional<Texture> color) :
    width_{width}, height_{height}, depth_{std::move(depth)}, color_{std::move(color)}
{
    initialize(false);
}

Framebuffer::Framebuffer(std::uint32_t width, std::uint32_t height, Renderbuffer depth, std::optional<Texture> color) :
    width_{width}, height_{height}, depth_{std::move(depth)}, color_{std::move(color)}
{
    initialize(true);
}

void Framebuffer::initialize(bool use_depth_renderbuffer)
{
    glCreateFramebuffers(1, &id_);
    assert(id_ != 0);
    if (color_)
    {
        glNamedFramebufferTexture(id_, GL_COLOR_ATTACHMENT0, color_.value().id(), 0);
    }
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if (use_depth_renderbuffer)
    {
        glNamedFramebufferRenderbuffer(id_, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, std::get<Renderbuffer>(depth_).id());
    }
    else
    {
        glNamedFramebufferTexture(id_, GL_DEPTH_ATTACHMENT, std::get<Texture>(depth_).id(), 0);
    }

    const auto status = glCheckNamedFramebufferStatus(id_, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "GL Error: " << glGetError() << "\n";
        throw std::runtime_error("Framebuffer incomplete");
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept :
    width_{other.width_}, height_{other.height_}, id_{other.id_}, depth_{std::move(other.depth_)}, color_{std::move(
                                                                                                       other.color_)}
{
    other.id_ = 0;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    std::swap(width_, other.width_);
    std::swap(height_, other.height_);
    std::swap(id_, other.id_);
    std::swap(depth_, other.depth_);
    std::swap(color_, other.color_);

    return *this;
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &id_);
}

void Framebuffer::clear()
{
    glClearNamedFramebufferfv(id_, GL_COLOR, 0, clear_color_.data());
    glClearNamedFramebufferfv(id_, GL_DEPTH, 0, &clear_depth_);

    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
    glViewport(0, 0, width_, height_);
    clear();
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::uint32_t Framebuffer::width() const
{
    return width_;
}

std::uint32_t Framebuffer::height() const
{
    return height_;
}

std::uint32_t Framebuffer::id() const
{
    return id_;
}

std::uint32_t Framebuffer::color_id() const
{
    return color_.value().id();
}

std::uint32_t Framebuffer::depth_id() const
{
    return std::visit([](const auto& depth) { return depth.id(); }, depth_);
}

void Framebuffer::bind_color(std::uint32_t texture_unit)
{
    color_.value().bind(texture_unit);
}

void Framebuffer::bind_depth_texture(std::uint32_t texture_unit)
{
    std::get<Texture>(depth_).bind(texture_unit);
}

} // namespace gl