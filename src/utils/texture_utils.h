#pragma once

#include "render/texture_manager.h"

namespace TextureUtils
{
    TextureData loadData(std::string_view path);
    GLenum getFormat(int channels);
    GLenum getInternalFormat(int channels);
}
