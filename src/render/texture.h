#pragma once

class Texture
{
public:

    Texture(GLuint id = 0, bool transparent = false)
        : _id(id), _transparent(transparent) {}
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    void bind(GLuint slot = 0) const;

    GLuint getID() const { return _id; }
    bool isTransparent() const { return _transparent; }

private:

    GLuint _id;
    bool _transparent;
};
