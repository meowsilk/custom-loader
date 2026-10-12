#include "ui/widgets/product_poster.hxx"

namespace loader::ui
{
namespace
{

constexpr float kAspect = 1.5F;
constexpr float kCornerRadius = 20.0F;

}

ProductPoster::ProductPoster(UiContext& context, render::ArtImage image, float width)
    : Widget(context), image_(image)
{
    style().width = width;
    style().height = width * kAspect;
}

void ProductPoster::paintSelf(PaintContext& context)
{
    const core::Rect box = bounds();
    const render::Corners corners = render::Corners::uniform(kCornerRadius);

    context.useBack();
    context.draw().addShadow(
        {box, kCornerRadius, {0.0F, 14.0F}, 20.0F, core::Color::fromHex(0x000000, 0.5F)});
    context.draw().addArt({box, corners, image_, 0.0F, 0.0F, 0.0F});
    context.draw().addRect(
        {box, corners, core::Color::transparent(), core::Color::fromHex(0xFFFFFF, 0.2F), 1.0F});
}

}
