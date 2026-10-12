#include "render/draw_list.hxx"

#include "text/msdf_generator.hxx"

#include <algorithm>
#include <cmath>
#include <tuple>

namespace loader::render
{
namespace
{

constexpr float kShadowExtentSigmas = 3.0F;
constexpr float kAntialiasMargin = 1.0F;

Float4 toFloat4(const core::Color& color)
{
    const core::Color premultiplied = color.premultiplied();
    return {premultiplied.r, premultiplied.g, premultiplied.b, premultiplied.a};
}

Float4 toFloat4(const core::Rect& rect, float scale)
{
    return {rect.x * scale, rect.y * scale, rect.width * scale, rect.height * scale};
}

Float4 toFloat4(const Corners& corners, float scale)
{
    return {corners.topLeft * scale,
            corners.topRight * scale,
            corners.bottomRight * scale,
            corners.bottomLeft * scale};
}

float clampRadius(float radius, const core::Rect& rect)
{
    return std::clamp(radius, 0.0F, std::min(rect.width, rect.height) * 0.5F);
}

Corners clampCorners(const Corners& corners, const core::Rect& rect)
{
    return {clampRadius(corners.topLeft, rect),
            clampRadius(corners.topRight, rect),
            clampRadius(corners.bottomRight, rect),
            clampRadius(corners.bottomLeft, rect)};
}

}

void DrawList::begin(float scale, core::SizeI stagePixels)
{
    scale_ = scale;
    stagePixels_ = stagePixels;
    for (auto& layer : layers_)
        layer.clear();
    currentLayer_ = Layer::Back;
    offset_ = {};
    opacity_ = 1.0F;
    transformStack_.clear();
    windowMask_.reset();
    backdropField_ = {};
    clips_.clear();
    clips_.push_back({{0.0F,
                       0.0F,
                       static_cast<float>(stagePixels.width) / scale,
                       static_cast<float>(stagePixels.height) / scale},
                      0.0F});
}

void DrawList::applyClip(ShapeInstance& instance) const
{
    const Clip clip = currentClip();
    instance.clip = toFloat4(clip.rect, scale_);
    instance.params0.y = clip.cornerRadius * scale_;
}

void DrawList::addRect(const RectStyle& style)
{
    ShapeInstance instance;
    instance.rect = toFloat4(style.rect.translated(offset_), scale_);
    instance.radii = toFloat4(clampCorners(style.corners, style.rect), scale_);
    instance.fill = toFloat4(style.fill);
    instance.border = toFloat4(style.border);
    instance.params0.x =
        style.borderWidth > 0.0F ? std::max(1.0F, std::round(style.borderWidth * scale_)) : 0.0F;
    instance.params0.z = kAntialiasMargin;
    instance.params0.w = static_cast<float>(ShapeKind::Rect);
    applyClip(instance);
    push(instance);
}

void DrawList::addShadow(const ShadowStyle& style)
{
    const core::Rect caster = style.caster.translated(style.offset + offset_);

    ShapeInstance instance;
    instance.rect = toFloat4(caster, scale_);
    instance.radii = {clampRadius(style.cornerRadius, caster) * scale_, 0.0F, 0.0F, 0.0F};
    instance.fill = toFloat4(style.color);
    instance.params0.z = std::ceil(style.sigma * scale_ * kShadowExtentSigmas) + kAntialiasMargin;
    instance.params0.w = static_cast<float>(ShapeKind::Shadow);
    instance.params1.x = style.sigma * scale_;
    applyClip(instance);
    push(instance);
}

void DrawList::addGlass(const GlassStyle& style)
{
    ShapeInstance instance;
    instance.rect = toFloat4(style.rect.translated(offset_), scale_);
    instance.radii = toFloat4(clampCorners(style.corners, style.rect), scale_);
    instance.fill = toFloat4(style.tint);
    instance.border = toFloat4(style.rim);
    instance.params0.x = style.bevel * scale_;
    instance.params0.z = kAntialiasMargin;
    instance.params0.w = static_cast<float>(ShapeKind::Glass);
    instance.params1 = {style.refraction * scale_, style.dispersion, style.specular, style.frost * scale_};
    instance.params2.y = style.blur * scale_;
    applyClip(instance);
    push(instance);
}

void DrawList::setBackdropField(const BackdropField& field)
{
    backdropField_ = field;
    backdropField_.position = field.position * scale_;
    backdropField_.trail = field.trail * scale_;
    backdropField_.velocity = field.velocity * scale_;
    backdropField_.radius = field.radius * scale_;
    for (Ripple& ripple : backdropField_.ripples)
        ripple.origin = ripple.origin * scale_;
}

void DrawList::addHero(const HeroStyle& style)
{
    ShapeInstance instance;
    instance.rect = toFloat4(style.rect.translated(offset_), scale_);
    instance.radii = toFloat4(clampCorners(style.corners, style.rect), scale_);
    instance.fill = {style.tint.r, style.tint.g, style.tint.b, style.opacity};
    instance.params0.z = kAntialiasMargin;
    instance.params0.w = static_cast<float>(ShapeKind::Hero);
    instance.params1 = {style.seed, style.brightness, style.patternScale * scale_, style.fade};
    applyClip(instance);
    push(instance);
}

void DrawList::addArt(const ArtStyle& style)
{
    ShapeInstance instance;
    instance.rect = toFloat4(style.rect.translated(offset_), scale_);
    instance.radii = toFloat4(clampCorners(style.corners, style.rect), scale_);
    instance.params0.z = kAntialiasMargin;
    instance.params0.w = static_cast<float>(ShapeKind::Art);
    instance.params1 = {static_cast<float>(style.image), style.hover, style.scrim, style.dim};
    applyClip(instance);
    push(instance);
}

void DrawList::addTextWithShadow(const text::ShapedText& text, core::Vec2 origin, const core::Color& color)
{
    addText(text, origin + core::Vec2{0.0F, 1.5F}, core::Color::fromHex(0x000000, 0.5F * color.a));
    addText(text, origin, color);
}

void DrawList::addFlag(const FlagStyle& style)
{
    ShapeInstance instance;
    instance.rect = toFloat4(style.rect.translated(offset_), scale_);
    instance.radii = toFloat4(clampCorners(style.corners, style.rect), scale_);
    instance.params0.z = kAntialiasMargin;
    instance.params0.w = static_cast<float>(ShapeKind::Flag);
    instance.params1 = {static_cast<float>(style.image), style.brightness, 0.0F, 0.0F};
    applyClip(instance);
    push(instance);
}

void DrawList::addText(const text::ShapedText& text, core::Vec2 origin, const core::Color& color)
{
    if (glyphCache_ == nullptr || text.font == nullptr)
        return;

    origin = origin + offset_;
    const float emPixels = text.size * scale_;
    const float baseline = std::round((origin.y + text.baselineOffset()) * scale_);
    const float screenRange = std::max(1.0F,
                                       static_cast<float>(text::kGlyphRangePixels) * emPixels /
                                           static_cast<float>(text::kGlyphEmPixels));

    for (const text::ShapedGlyph& glyph : text.glyphs)
    {
        const text::GlyphMetrics& metrics = glyphCache_->glyph(*text.font, glyph.glyphId);
        if (!metrics.visible)
            continue;

        const float penX = (origin.x + glyph.x) * scale_;
        const float offsetY = glyph.y * scale_;
        const float left = penX + metrics.planeLeft * emPixels;
        const float right = penX + metrics.planeRight * emPixels;
        const float top = baseline + offsetY - metrics.planeTop * emPixels;
        const float bottom = baseline + offsetY - metrics.planeBottom * emPixels;

        ShapeInstance instance;
        instance.rect = {left, top, right - left, bottom - top};
        instance.radii = {metrics.u0, metrics.v0, metrics.u1, metrics.v1};
        instance.fill = toFloat4(color);
        instance.params0.w = static_cast<float>(ShapeKind::Glyph);
        instance.params1.x = screenRange;
        applyClip(instance);
        push(instance);
    }
}

void DrawList::pushClip(core::Rect rect, float cornerRadius)
{
    clips_.push_back({currentClip().rect.intersection(rect.translated(offset_)), cornerRadius});
}

void DrawList::popClip()
{
    if (clips_.size() > 1)
        clips_.pop_back();
}

void DrawList::pushTransform(core::Vec2 offset, float opacity)
{
    transformStack_.emplace_back(offset_, opacity_);
    offset_ = offset_ + offset;
    opacity_ *= opacity;
}

void DrawList::popTransform()
{
    if (transformStack_.empty())
        return;
    std::tie(offset_, opacity_) = transformStack_.back();
    transformStack_.pop_back();
}

void DrawList::setWindowShape(core::Rect rect, float cornerRadius)
{
    ShapeInstance instance;
    instance.rect = toFloat4(rect, scale_);
    instance.radii = toFloat4(clampCorners(Corners::uniform(cornerRadius), rect), scale_);
    instance.params0.z = kAntialiasMargin;
    instance.params0.w = static_cast<float>(ShapeKind::Mask);
    windowMask_ = instance;
}

}
