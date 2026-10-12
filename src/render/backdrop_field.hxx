#pragma once

#include "core/geometry.hxx"

#include <array>
#include <cstddef>

namespace loader::render
{

inline constexpr std::size_t kMaxRipples = 4;

struct Ripple
{
    core::Vec2 origin;
    float age = 0.0F;
    float amplitude = 0.0F;
};

struct BackdropField
{
    core::Vec2 position;
    core::Vec2 trail;
    core::Vec2 velocity;
    float presence = 0.0F;
    float radius = 1.0F;
    std::array<Ripple, kMaxRipples> ripples{};
};

}
