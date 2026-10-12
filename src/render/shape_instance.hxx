#pragma once

#include <cstdint>

namespace loader::render
{

enum class ShapeKind : std::uint32_t
{
    Rect = 0,
    Shadow = 1,
    Mask = 2,
    Glyph = 3,
    Glass = 4,
    Hero = 5,
    Art = 6,
    Flag = 7,
};

struct Float4
{
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    float w = 0.0F;
};

struct ShapeInstance
{
    Float4 rect;
    Float4 radii;
    Float4 fill;
    Float4 border;
    Float4 clip;
    Float4 params0;
    Float4 params1;
    Float4 params2{1.0F, 0.0F, 0.0F, 0.0F};
};

static_assert(sizeof(ShapeInstance) == 128);
static_assert(sizeof(ShapeInstance) % 16 == 0);

}
