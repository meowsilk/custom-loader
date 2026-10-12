#pragma once

#include "text/font_library.hxx"

#include <cstdint>
#include <optional>
#include <vector>

namespace loader::text
{

inline constexpr int kGlyphEmPixels = 64;
inline constexpr int kGlyphRangePixels = 8;

struct GlyphBitmap
{
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba;
    float planeLeft = 0.0F;
    float planeRight = 0.0F;
    float planeTop = 0.0F;
    float planeBottom = 0.0F;
};

std::optional<GlyphBitmap> generateGlyphBitmap(const FontInstance& font, std::uint32_t glyphId);

}
