#include "text/glyph_cache.hxx"

#include "text/msdf_generator.hxx"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace loader::text
{
namespace
{

constexpr int kCellSpacing = 1;

}

GlyphCache::GlyphCache() : pixels_(static_cast<std::size_t>(kAtlasSize) * kAtlasSize * kBytesPerPixel, 0) {}

const GlyphMetrics& GlyphCache::glyph(const FontInstance& font, std::uint32_t glyphId)
{
    const Key key{&font, glyphId};
    if (const auto found = glyphs_.find(key); found != glyphs_.end())
        return found->second;

    GlyphMetrics metrics;
    if (const std::optional<GlyphBitmap> bitmap = generateGlyphBitmap(font, glyphId))
    {
        const AtlasRegion region = allocate(bitmap->width, bitmap->height);
        const std::size_t rowBytes = static_cast<std::size_t>(bitmap->width) * kBytesPerPixel;
        for (int row = 0; row < bitmap->height; ++row)
        {
            const std::size_t destination = (static_cast<std::size_t>(region.top + row) * kAtlasSize +
                                             static_cast<std::size_t>(region.left)) *
                                            kBytesPerPixel;
            std::memcpy(
                &pixels_[destination], &bitmap->rgba[static_cast<std::size_t>(row) * rowBytes], rowBytes);
        }
        markDirty(region);

        constexpr float inverseSize = 1.0F / static_cast<float>(kAtlasSize);
        metrics.visible = true;
        metrics.u0 = static_cast<float>(region.left) * inverseSize;
        metrics.v0 = static_cast<float>(region.top) * inverseSize;
        metrics.u1 = static_cast<float>(region.right) * inverseSize;
        metrics.v1 = static_cast<float>(region.bottom) * inverseSize;
        metrics.planeLeft = bitmap->planeLeft;
        metrics.planeTop = bitmap->planeTop;
        metrics.planeRight = bitmap->planeRight;
        metrics.planeBottom = bitmap->planeBottom;
    }
    return glyphs_.emplace(key, metrics).first->second;
}

AtlasRegion GlyphCache::allocate(int width, int height)
{
    if (cursorX_ + width > kAtlasSize)
    {
        cursorY_ += rowHeight_ + kCellSpacing;
        cursorX_ = 0;
        rowHeight_ = 0;
    }
    if (width > kAtlasSize || cursorY_ + height > kAtlasSize)
        throw std::runtime_error("glyph atlas exhausted");

    const AtlasRegion region{cursorX_, cursorY_, cursorX_ + width, cursorY_ + height};
    cursorX_ += width + kCellSpacing;
    rowHeight_ = std::max(rowHeight_, height);
    return region;
}

void GlyphCache::markDirty(const AtlasRegion& region)
{
    if (!dirty_)
    {
        dirty_ = region;
        return;
    }
    dirty_->left = std::min(dirty_->left, region.left);
    dirty_->top = std::min(dirty_->top, region.top);
    dirty_->right = std::max(dirty_->right, region.right);
    dirty_->bottom = std::max(dirty_->bottom, region.bottom);
}

std::optional<AtlasRegion> GlyphCache::takeDirtyRegion()
{
    return std::exchange(dirty_, std::nullopt);
}

}
