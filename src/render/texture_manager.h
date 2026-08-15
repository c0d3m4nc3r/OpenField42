#pragma once

constexpr unsigned int INVALID_TEXTURE_ID = -1;

struct TextureHandle
{
    unsigned int id = INVALID_TEXTURE_ID;

    bool isValid() const { return id != INVALID_TEXTURE_ID; }
};

class Texture;
class TextureManager
{
public:

    TextureHandle load(const std::string& path);
    TextureHandle loadAtlas(const std::vector<std::string>& paths, int tile_w, int tile_h, int channels = 4);
    void clear();

    Texture& get(const TextureHandle& handle);
    Texture& getDefault();

private:

    std::unordered_map<std::string, TextureHandle> _path_to_handle;
    std::vector<Texture> _textures;
    std::unique_ptr<Texture> _default_tex;

};
