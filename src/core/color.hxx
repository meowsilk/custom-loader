#pragma once

#include <cstdint>

namespace loader::core
{

struct Color
{
    float r{};
    float g{};
    float b{};
    float a{1.0F};

    static constexpr Color transparent() { return {0.0F, 0.0F, 0.0F, 0.0F}; }

    static constexpr Color fromHex(std::uint32_t rgb, float alpha = 1.0F)
    {
        return {static_cast<float>((rgb >> 16) & 0xFFU) / 255.0F,
                static_cast<float>((rgb >> 8) & 0xFFU) / 255.0F,
                static_cast<float>(rgb & 0xFFU) / 255.0F,
                alpha};
    }

    constexpr Color withAlpha(float alpha) const { return {r, g, b, alpha}; }
    constexpr Color scaledAlpha(float factor) const { return {r, g, b, a * factor}; }
    constexpr Color premultiplied() const { return {r * a, g * a, b * a, a}; }

    static constexpr Color lerp(Color from, Color to, float t)
    {
        return {from.r + (to.r - from.r) * t,
                from.g + (to.g - from.g) * t,
                from.b + (to.b - from.b) * t,
                from.a + (to.a - from.a) * t};
    }

    friend constexpr bool operator==(Color, Color) = default;
};

}
