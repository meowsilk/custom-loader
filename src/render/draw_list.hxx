#pragma once

#include "core/color.hxx"
#include "core/geometry.hxx"
#include "render/art_images.hxx"
#include "render/backdrop_field.hxx"
#include "render/shape_instance.hxx"
#include "text/glyph_cache.hxx"
#include "text/shaper.hxx"

#include <array>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace loader::render
{

struct Corners
{
    float topLeft = 0.0F;
    float topRight = 0.0F;
    float bottomRight = 0.0F;
    float bottomLeft = 0.0F;

    static constexpr Corners uniform(float radius) { return {radius, radius, radius, radius}; }
};

struct RectStyle
{
    core::Rect rect;
    Corners corners;
    core::Color fill = core::Color::transparent();
    core::Color border = core::Color::transparent();
    float borderWidth = 0.0F;
};

struct ShadowStyle
{
    core::Rect caster;
    float cornerRadius = 0.0F;
    core::Vec2 offset;
    float sigma = 1.0F;
    core::Color color;
};

struct GlassStyle
{
    core::Rect rect;
    Corners corners;
    core::Color tint;
    core::Color rim;
    float bevel = 8.0F;
    float refraction = 8.0F;
    float dispersion = 0.015F;
    float specular = 0.3F;
    float frost = 1.0F;
    float blur = 0.0F;
};

struct HeroStyle
{
    core::Rect rect;
    Corners corners;
    core::Color tint = core::Color::fromHex(0xFFFFFF);
    float seed = 0.0F;
    float brightness = 0.6F;
    float opacity = 1.0F;
    float fade = 0.0F;
    float patternScale = 260.0F;
};

struct ArtStyle
{
    core::Rect rect;
    Corners corners;
    ArtImage image = ArtImage::CounterStrike2;
    float hover = 0.0F;
    float dim = 0.0F;
    float scrim = 0.8F;
};

struct FlagStyle
{
    core::Rect rect;
    Corners corners;
    FlagImage image = FlagImage::English;
    float brightness = 1.0F;
};

enum class Layer
{
    Back = 0,
    Glass = 1,
    GlassOverGlass = 2,
    Front = 3,
    OverlayGlass = 4,
    OverlayFront = 5,
};

inline constexpr std::size_t kLayerCount = 6;

constexpr bool isGlassLayer(Layer layer)
{
    return layer == Layer::Glass || layer == Layer::GlassOverGlass || layer == Layer::OverlayGlass;
}

class DrawList
{
public:
    void setGlyphCache(text::GlyphCache& cache) { glyphCache_ = &cache; }
    void begin(float scale, core::SizeI stagePixels);

    void addRect(const RectStyle& style);
    void addShadow(const ShadowStyle& style);
    void addGlass(const GlassStyle& style);
    void addHero(const HeroStyle& style);
    void addArt(const ArtStyle& style);
    void addFlag(const FlagStyle& style);
    void setLayer(Layer layer) { currentLayer_ = layer; }
    void pushTransform(core::Vec2 offset, float opacity);
    void popTransform();
    void addText(const text::ShapedText& text, core::Vec2 origin, const core::Color& color);
    void addTextWithShadow(const text::ShapedText& text, core::Vec2 origin, const core::Color& color);
    void pushClip(core::Rect rect, float cornerRadius);
    void popClip();
    void setWindowShape(core::Rect rect, float cornerRadius);
    void setBackdropField(const BackdropField& field);

    std::span<const ShapeInstance> layer(Layer layer) const
    {
        return layers_[static_cast<std::size_t>(layer)];
    }
    std::optional<ShapeInstance> windowMask() const { return windowMask_; }
    const BackdropField& backdropField() const { return backdropField_; }

private:
    struct Clip
    {
        core::Rect rect;
        float cornerRadius = 0.0F;
    };

    Clip currentClip() const { return clips_.back(); }
    void applyClip(ShapeInstance& instance) const;
    void push(ShapeInstance instance)
    {
        instance.params2.x = opacity_;
        layers_[static_cast<std::size_t>(currentLayer_)].push_back(instance);
    }

    text::GlyphCache* glyphCache_ = nullptr;
    float scale_ = 1.0F;
    core::SizeI stagePixels_;
    std::array<std::vector<ShapeInstance>, kLayerCount> layers_;
    Layer currentLayer_ = Layer::Back;
    core::Vec2 offset_;
    float opacity_ = 1.0F;
    std::vector<std::pair<core::Vec2, float>> transformStack_;
    std::vector<Clip> clips_;
    std::optional<ShapeInstance> windowMask_;
    BackdropField backdropField_;
};

}
