#pragma once

namespace GLUtils
{
    unsigned int compileShader(const char* src, unsigned int type);
    unsigned int linkProgram(unsigned int vert_shader, unsigned int frag_shader);
}