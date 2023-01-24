#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <cassert>
#include <exception>
#include <glm/glm.hpp>

namespace gl
{

Texture::Texture(std::uint32_t width, std::uint32_t height, Attributes attributes) :
    width_{width}, height_{height}, attributes_{attributes}
{
    initialize();
}

void Texture::initialize()
{
    set_texture_parameters();

    if (attributes_.generate_mipmap && attributes_.target != GL_TEXTURE_CUBE_MAP)
    {
        const float min_dimension{static_cast<float>(std::min(width_, height_))};
        attributes_.mip_levels = static_cast<GLsizei>(glm::ceil(glm::log2(min_dimension)));
    }

    if (attributes_.target == GL_TEXTURE_2D_ARRAY)
    {
        glTextureStorage3D(id_, attributes_.mip_levels, attributes_.internal_format, width_, height_,
                           attributes_.layers.value());
    }
    else
    {
        glTextureStorage2D(id_, attributes_.mip_levels, attributes_.internal_format, width_, height_);
    }
}

void Texture::set_texture_parameters()
{
    glCreateTextures(attributes_.target, 1, &id_);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, attributes_.wrap_s);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, attributes_.wrap_t);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, attributes_.wrap_r);
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, attributes_.min_filter);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, attributes_.mag_filter);
}

Texture::Texture(std::uint32_t width, std::uint32_t height) : width_{width}, height_{height}
{
    initialize();
}

Texture::Texture(Texture&& other) noexcept :
    width_{other.width_}, height_{other.height_}, attributes_{other.attributes_}, id_{other.id_}
{
    other.id_ = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    std::swap(width_, other.width_);
    std::swap(height_, other.height_);
    std::swap(attributes_, other.attributes_);
    std::swap(id_, other.id_);
    return *this;
}

Texture::~Texture()
{
    glDeleteTextures(1, &id_);
}

void Texture::bind(std::uint32_t unit)
{
    glBindTextureUnit(unit, id_);
}

void Texture::bind_image(std::uint32_t unit, GLenum access)
{
    glBindImageTexture(unit, id_, 0, GL_FALSE, 0, access, attributes_.internal_format);
}

std::uint32_t Texture::id() const
{
    return id_;
}

std::uint32_t Texture::width() const
{
    return width_;
}

std::uint32_t Texture::height() const
{
    return height_;
}

void Texture::set_border_color(const std::array<float, 4> border_color)
{
    if (attributes_.wrap_s != GL_CLAMP_TO_BORDER || attributes_.wrap_t != GL_CLAMP_TO_BORDER)
    {
        throw std::invalid_argument("Texture wrapping mode is not set to clamp to border");
    }

    glTextureParameterfv(id_, GL_TEXTURE_BORDER_COLOR, border_color.data());
}

void Texture::copy_image(std::string_view filename, bool flip_on_load)
{
    int width{0};
    int height{0};
    int number_of_channels{0};
    stbi_set_flip_vertically_on_load(flip_on_load);
    unsigned char* data = stbi_load(filename.data(), &width, &height, &number_of_channels, 0);
    assert(data != nullptr);
    if (number_of_channels == 3)
    {
        attributes_.pixel_data_format = GL_RGB;
    }
    else if (number_of_channels == 1)
    {
        attributes_.pixel_data_format = GL_RED;
    }

    copy_image(data, width, height);
    stbi_image_free(data);

    if (flip_on_load)
    {
        stbi_set_flip_vertically_on_load(!flip_on_load);
    }
}

void Texture::generate_mipmap()
{
    if (attributes_.generate_mipmap && attributes_.target != GL_TEXTURE_CUBE_MAP)
    {
        glGenerateTextureMipmap(id_);
    }
}

void Texture::load_cubemap(const std::vector<std::string_view>& filenames, bool flip_on_load)
{
    assert(attributes_.target == GL_TEXTURE_CUBE_MAP);
    stbi_set_flip_vertically_on_load(flip_on_load);
    for (std::size_t face = 0; face < filenames.size(); ++face)
    {
        int width{0};
        int height{0};
        int number_of_channels{0};
        unsigned char* data = stbi_load(filenames[face].data(), &width, &height, &number_of_channels, 0);
        assert(data != nullptr);
        if (number_of_channels == 3)
        {
            attributes_.pixel_data_format = GL_RGB;
        }
        else if (number_of_channels == 1)
        {
            attributes_.pixel_data_format = GL_RED;
        }
        glTextureSubImage3D(id_, 0, 0, 0, face, width, height, 1, attributes_.pixel_data_format,
                            attributes_.pixel_data_type, data);
        stbi_image_free(data);
    }

    if (flip_on_load)
    {
        stbi_set_flip_vertically_on_load(!flip_on_load);
    }
}

void Texture::load_array_texture(const std::vector<std::string_view>& filenames, bool flip_on_load)
{
    if (attributes_.target != GL_TEXTURE_2D_ARRAY)
    {
        throw std::invalid_argument("Expects a GL_TEXTURE_2D_ARRAY");
    }

    if (attributes_.layers.value() != static_cast<GLsizei>(filenames.size()))
    {
        throw std::invalid_argument("Number of images is incompatible with the number of layers of the array texture");
    }

    stbi_set_flip_vertically_on_load(flip_on_load);
    for (std::size_t layer = 0; layer < filenames.size(); ++layer)
    {
        int width{0};
        int height{0};
        int number_of_channels{0};
        unsigned char* data = stbi_load(filenames[layer].data(), &width, &height, &number_of_channels, 0);
        assert(data != nullptr);
        // For array textures, all textures must have the same pixel data format
        if (number_of_channels == 3 && attributes_.pixel_data_format != GL_RGB)
        {
            throw std::invalid_argument("Image has incompatible data format of type GL_RGB");
        }
        else if (number_of_channels == 1 && attributes_.pixel_data_format != GL_RED)
        {
            throw std::invalid_argument("Image has incompatible data format of type GL_RED");
        }
        glTextureSubImage3D(id_, 0, 0, 0, layer, width, height, 1, attributes_.pixel_data_format,
                            attributes_.pixel_data_type, data);
        stbi_image_free(data);
    }

    generate_mipmap();

    if (flip_on_load)
    {
        stbi_set_flip_vertically_on_load(!flip_on_load);
    }
}

Texture create_texture_from_file(std::string_view filename, Texture::Attributes attributes, bool flip_on_load)
{
    int width{0};
    int height{0};
    int number_of_channels{0};
    stbi_set_flip_vertically_on_load(flip_on_load);
    unsigned char* data = stbi_load(filename.data(), &width, &height, &number_of_channels, 0);
    assert(data != nullptr);
    if (number_of_channels == 3)
    {
        attributes.pixel_data_format = GL_RGB;
    }
    else if (number_of_channels == 1)
    {
        attributes.pixel_data_format = GL_RED;
    }
    Texture texture{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), attributes};
    texture.copy_image(data, width, height);
    stbi_image_free(data);

    if (flip_on_load)
    {
        stbi_set_flip_vertically_on_load(!flip_on_load);
    }

    return texture;
}

} // namespace gl