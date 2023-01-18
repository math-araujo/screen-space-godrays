#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include <glad/glad.h>

namespace gl
{

class Texture
{
public:
    struct Attributes
    {
        GLenum target{GL_TEXTURE_2D};
        GLint wrap_s{GL_REPEAT};
        GLint wrap_t{GL_REPEAT};
        GLint wrap_r{GL_CLAMP_TO_EDGE};
        GLint min_filter{GL_LINEAR};
        GLint mag_filter{GL_LINEAR};
        GLenum internal_format{GL_RGBA8};
        GLenum pixel_data_format{GL_RGBA};
        GLenum pixel_data_type{GL_UNSIGNED_BYTE};
        bool generate_mipmap{false};
        GLsizei mip_levels{1};
        std::optional<GLsizei> layers{};
    };

    Texture(std::uint32_t width, std::uint32_t height, Attributes attributes);
    Texture(std::uint32_t width, std::uint32_t height);

    Texture(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&& other) noexcept;
    ~Texture();

    template <typename T>
    void copy_image(const T* image_data, std::int32_t width, std::int32_t height);

    void copy_image(std::string_view filename, bool flip_on_load = true);
    void load_cubemap(const std::vector<std::string_view>& filenames, bool flip_on_load = true);
    void load_array_texture(const std::vector<std::string_view>& filenames, bool flip_on_load = true);
    void bind(std::uint32_t unit);
    void bind_image(std::uint32_t unit, GLenum access = GL_READ_WRITE);
    std::uint32_t id() const;
    std::uint32_t width() const;
    std::uint32_t height() const;

private:
    std::uint32_t width_;
    std::uint32_t height_;
    Attributes attributes_{};
    std::uint32_t id_{0};

    void initialize();
    void set_texture_parameters();
    void generate_mipmap();
};

Texture create_texture_from_file(std::string_view filename, Texture::Attributes attributes = {},
                                 bool flip_on_load = true);

} // namespace gl

#include "texture.inl"

#endif // TEXTURE_HPP