#pragma once

#include <Windows.h>

#include <span>

namespace loader::gpu
{

struct ShaderBytecode
{
    std::span<const BYTE> bytes;
};

namespace shaders
{

ShaderBytecode uiVertex();
ShaderBytecode uiPixel();

}

}
