#pragma once

#include "render/texture_manager.h"

namespace TextureUtils
{
    TextureData loadData(const std::string& path);
    GLenum getFormat(int channels);
    GLenum getInternalFormat(int channels);
}
