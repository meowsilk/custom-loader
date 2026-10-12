#pragma once

#include "text/font_library.hxx"

#include <cstdint>
#include <string_view>
#include <vector>

namespace loader::text
{

struct TextStyle
{
    FontFamily family = FontFamily::Inter;
    float size = 14.0F;
    float weight = 400.0F;
    float lineHeight = 20.0F;
    float tracking = 0.0F;
};

struct ShapedGlyph
{
    std::uint32_t glyphId = 0;
    float x = 0.0F;
    float y = 0.0F;
    float advance = 0.0F;
    std::uint32_t cluster = 0;
};

struct ShapedText
{
    std::vector<ShapedGlyph> glyphs;
    FontInstance* font = nullptr;
    float size = 0.0F;
    float width = 0.0F;
    float ascent = 0.0F;
    float descent = 0.0F;
    float lineHeight = 0.0F;

    float baselineOffset() const { return (lineHeight - (ascent + descent)) * 0.5F + ascent; }
};

ShapedText shapeText(FontLibrary& fonts, const TextStyle& style, std::u32string_view text);

}
