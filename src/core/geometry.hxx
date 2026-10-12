#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace loader::core
{

struct Vec2
{
    float x{};
    float y{};

    friend constexpr bool operator==(Vec2, Vec2) = default;

    friend constexpr Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
    friend constexpr Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
    friend constexpr Vec2 operator*(Vec2 a, float s) { return {a.x * s, a.y * s}; }
};

struct Size
{
    float width{};
    float height{};

    friend constexpr bool operator==(Size, Size) = default;
};

struct Insets
{
    float left{};
    float top{};
    float right{};
    float bottom{};

    static constexpr Insets uniform(float value) { return {value, value, value, value}; }

    constexpr float horizontal() const { return left + right; }
    constexpr float vertical() const { return top + bottom; }

    friend constexpr bool operator==(Insets, Insets) = default;
};

struct Rect
{
    float x{};
    float y{};
    float width{};
    float height{};

    constexpr float right() const { return x + width; }
    constexpr float bottom() const { return y + height; }
    constexpr Vec2 origin() const { return {x, y}; }
    constexpr Size size() const { return {width, height}; }
    constexpr Vec2 center() const { return {x + width * 0.5F, y + height * 0.5F}; }
    constexpr bool empty() const { return width <= 0.0F || height <= 0.0F; }

    constexpr bool contains(Vec2 point) const
    {
        return point.x >= x && point.x < right() && point.y >= y && point.y < bottom();
    }

    constexpr Rect inset(Insets insets) const
    {
        return {x + insets.left,
                y + insets.top,
                std::max(0.0F, width - insets.horizontal()),
                std::max(0.0F, height - insets.vertical())};
    }

    constexpr Rect translated(Vec2 offset) const { return {x + offset.x, y + offset.y, width, height}; }

    constexpr Rect intersection(Rect other) const
    {
        const float left = std::max(x, other.x);
        const float top = std::max(y, other.y);
        const float rightEdge = std::min(right(), other.right());
        const float bottomEdge = std::min(bottom(), other.bottom());
        return {left, top, std::max(0.0F, rightEdge - left), std::max(0.0F, bottomEdge - top)};
    }

    static constexpr Rect lerp(Rect from, Rect to, float t)
    {
        return {from.x + (to.x - from.x) * t,
                from.y + (to.y - from.y) * t,
                from.width + (to.width - from.width) * t,
                from.height + (to.height - from.height) * t};
    }

    friend constexpr bool operator==(Rect, Rect) = default;
};

struct SizeI
{
    std::int32_t width{};
    std::int32_t height{};

    friend constexpr bool operator==(SizeI, SizeI) = default;
};

}
