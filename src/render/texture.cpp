#include "render/texture.h"

Texture::~Texture()
{
    if (_id != 0) glDeleteTextures(1, &_id);
}

Texture::Texture(Texture&& other) noexcept
    : _id(other._id), _transparent(other._transparent)
{
    other._id = 0;
    other._transparent = false;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        if (_id != 0) glDeleteTextures(1, &_id);
        _id = other._id;
        _transparent = other._transparent;
        other._id = 0;
        other._transparent = false;
    }
    return *this;
}

void Texture::bind(GLuint slot) const
{
    glBindTextureUnit(slot, _id);
}
