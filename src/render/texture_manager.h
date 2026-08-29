#pragma once

#include "core/thread_safe_queue.h"
#include "render/texture.h"

constexpr unsigned int INVALID_TEXTURE_ID = -1;

struct TextureHandle
{
    unsigned int id = INVALID_TEXTURE_ID;

    bool isValid() const { return id != INVALID_TEXTURE_ID; }
};

struct TextureData
{
    std::vector<unsigned char> pixels;
    int width = 0;
    int height = 0;
    int channels = 0;
    bool is_valid = false;

    TextureHandle handle;
    std::string path;

    bool is_atlas_tile = false;
    int dst_x = 0;
    int dst_y = 0;
};

class TextureManager
{
public:

    TextureHandle load(const std::string& path);
    TextureHandle loadAtlas(const std::vector<std::string>& paths, int tile_w, int tile_h, int channels = 3);

    void clear();

    void updateGpuUploads(int max_uploads_per_frame);

    Texture& get(const TextureHandle& handle);
    Texture& getDefault();

    size_t count() const { return _textures.size(); }
    size_t getMemoryUsage() const { return _memory_usage; }

private:

    std::unordered_map<std::string, TextureHandle> _path_to_handle;
    std::vector<std::shared_ptr<Texture>> _textures;
    std::shared_ptr<Texture> _default_tex;

    ThreadSafeQueue<TextureData> _completed_uploads;
    std::unordered_map<unsigned int, int> _atlas_pending_tiles;

    static std::vector<unsigned char> conformTileData(const TextureData& src, int target_w, int target_h, int target_channels);
    static int calcMipLevels(int w, int h);
    void uploadNewTexture(const TextureData& task);
    void uploadAtlasTile(const TextureData& task);

    size_t _memory_usage = 0; // in bytes

};