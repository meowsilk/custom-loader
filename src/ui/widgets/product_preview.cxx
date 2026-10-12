#include "ui/widgets/product_preview.hxx"

#include "render/art_images.hxx"
#include "ui/animation/easing.hxx"

#include <algorithm>
#include <array>
#include <cmath>

namespace loader::ui
{
namespace
{

constexpr float kCardWidth = 66.0F;
constexpr float kCardHeight = 99.0F;
constexpr float kGap = 12.0F;
constexpr float kBobRange = 3.5F;
constexpr float kArchLift = 9.0F;
constexpr float kCornerRadius = 13.0F;
constexpr float kRevealStep = 0.11F;
constexpr float kRevealLength = 0.4F;
constexpr float kRevealRise = 16.0F;
constexpr float kLightStart = 0.7F;
constexpr float kLightStep = 0.45F;
constexpr float kLightLength = 0.5F;
constexpr float kRestingDim = 0.5F;
constexpr std::size_t kCount = 5;

constexpr std::array<render::ArtImage, kCount> kOrder = {render::ArtImage::CounterStrike2,
                                                         render::ArtImage::EscapeFromTarkov,
                                                         render::ArtImage::Fortnite,
                                                         render::ArtImage::ApexLegends,
                                                         render::ArtImage::DayZ};
constexpr std::array<float, kCount> kArch = {0.0F, 1.0F, 1.5F, 1.0F, 0.0F};

float span(float value, float start, float length)
{
    return std::clamp((value - start) / length, 0.0F, 1.0F);
}

float smooth(float t)
{
    return t * t * (3.0F - 2.0F * t);
}

}

ProductPreview::ProductPreview(UiContext& context) : Widget(context) {}

core::Size ProductPreview::measureContent()
{
    const float count = static_cast<float>(kCount);
    return {count * kCardWidth + (count - 1.0F) * kGap, kCardHeight + kArchLift + kBobRange * 2.0F};
}

bool ProductPreview::updateSelf(float deltaSeconds)
{
    time_ += deltaSeconds;
    return true;
}

void ProductPreview::paintSelf(PaintContext& context)
{
    const core::Rect box = bounds();
    const render::Corners corners = render::Corners::uniform(kCornerRadius);

    for (std::size_t i = 0; i < kCount; ++i)
    {
        const float index = static_cast<float>(i);
        const float reveal = animation::easing::enter(span(time_, kRevealStep * index, kRevealLength));
        if (reveal <= 0.01F)
            continue;

        const float lit = smooth(span(time_, kLightStart + kLightStep * index, kLightLength));
        const float bob = std::sin(time_ * 1.7F + index * 0.9F) * kBobRange;
        const float lift = kArch[i] * kArchLift * reveal;

        const core::Rect card{box.x + index * (kCardWidth + kGap),
                              box.y + kBobRange + kArchLift + bob - lift,
                              kCardWidth,
                              kCardHeight};

        context.draw().pushTransform({0.0F, kRevealRise * (1.0F - reveal)}, reveal);
        context.useBack();
        context.draw().addShadow(
            {card, kCornerRadius, {0.0F, 8.0F}, 12.0F, core::Color::fromHex(0x000000, 0.28F + 0.2F * lit)});
        context.draw().addArt({card, corners, kOrder[i], 0.0F, kRestingDim * (1.0F - lit), 0.0F});
        context.draw().addRect({card,
                                corners,
                                core::Color::transparent(),
                                core::Color::fromHex(0xFFFFFF, 0.12F + 0.3F * lit),
                                1.0F});
        context.draw().popTransform();
    }
}

}
