#pragma once

#include <string>
#include <vector>
#include <memory>

class Texture
{
public:

    Texture(GLuint id = 0);
    ~Texture();

    static std::shared_ptr<Texture> load(const std::string& path, bool generate_mipmaps = false);
    static std::shared_ptr<Texture> loadAtlas(const std::vector<std::string>& paths, int tile_w, int tile_h, int channels = 3, bool generate_mipmaps = false);

    void bind(int slot = 0);
    void unbind(int slot = 0);

    GLuint getID() const { return _id; }

    bool isTransparent() const { return _transparent; }

private:

    GLuint _id;

    bool _transparent = false;
};
