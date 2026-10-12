#include "ui/widgets/spinner.hxx"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace loader::ui
{

void paintSpinner(
    PaintContext& context, core::Vec2 center, float diameter, float phase, const core::Color& color)
{
    constexpr int kDots = 8;
    const float dotSize = std::max(2.0F, diameter * 0.16F);
    const float ringRadius = diameter * 0.5F - dotSize * 0.5F;

    context.useFront();
    for (int i = 0; i < kDots; ++i)
    {
        const float fraction = static_cast<float>(i) / static_cast<float>(kDots);
        const float angle = fraction * 2.0F * std::numbers::pi_v<float> - std::numbers::pi_v<float> * 0.5F;
        float trail = fraction - phase;
        trail -= std::floor(trail);
        const float alpha = 0.2F + 0.8F * (1.0F - trail);

        const core::Rect dot{center.x + std::cos(angle) * ringRadius - dotSize * 0.5F,
                             center.y + std::sin(angle) * ringRadius - dotSize * 0.5F,
                             dotSize,
                             dotSize};
        context.draw().addRect({dot, render::Corners::uniform(dotSize * 0.5F), color.scaledAlpha(alpha)});
    }
}

}
