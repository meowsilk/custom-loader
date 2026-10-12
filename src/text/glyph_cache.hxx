#pragma once

#include "text/font_library.hxx"

#include <compare>
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <vector>

namespace loader::text
{

struct GlyphMetrics
{
    bool visible = false;
    float u0 = 0.0F;
    float v0 = 0.0F;
    float u1 = 0.0F;
    float v1 = 0.0F;
    float planeLeft = 0.0F;
    float planeTop = 0.0F;
    float planeRight = 0.0F;
    float planeBottom = 0.0F;
};

struct AtlasRegion
{
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
};

class GlyphCache
{
public:
    static constexpr int kAtlasSize = 2048;
    static constexpr int kBytesPerPixel = 4;

    GlyphCache();

    const GlyphMetrics& glyph(const FontInstance& font, std::uint32_t glyphId);

    std::span<const std::uint8_t> pixels() const { return pixels_; }
    std::optional<AtlasRegion> takeDirtyRegion();

    std::size_t glyphCount() const { return glyphs_.size(); }
    int usedRows() const { return cursorY_ + rowHeight_; }

private:
    struct Key
    {
        const FontInstance* font;
        std::uint32_t glyphId;

        friend auto operator<=>(const Key&, const Key&) = default;
    };

    AtlasRegion allocate(int width, int height);
    void markDirty(const AtlasRegion& region);

    std::map<Key, GlyphMetrics> glyphs_;
    std::vector<std::uint8_t> pixels_;
    int cursorX_ = 0;
    int cursorY_ = 0;
    int rowHeight_ = 0;
    std::optional<AtlasRegion> dirty_;
};

}
